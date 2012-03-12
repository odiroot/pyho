#!/usr/bin/env python
import os
import sys
import subprocess
import atexit
import time

# Insert path to libs directory.
from utils import libs_to_path
libs_to_path()

from utils import optimizer_arguments, Timer, printf
from communication import LocalClientComm, NetworkClientComm
from genetic import CustomG1DList, CustomGSimpleGA, stats_step_callback
from genetic import AlleleG1DList


class MemoizedObjective(object):
    u"Memoized objective helper with generator result."
    memo = {}

    def __init__(self, comm):
        self.comm = comm

    def objective(self, chromosome):
        u"The Genetic Algorithm evaluation function"
        p = chromosome.getInternalList()
        key = tuple(p)

        if key in self.memo:  # We already had this task.
            yield self.memo[key]
        else:  # We have to compute a score.
            uid = id(chromosome)  # Unique id for messaging.
            sent = False  # Whether evaluation request is sent.
            while True:
                if not sent:
                    # Send asynchronous request for evaluation.
                    self.comm.request({"params": p}, uid, self.comm.DO_EVALUATION)
                    sent = True
                if sent:
                    # Try to retrieve evaluation score from the transport.
                    res = self.comm.response(uid, self.comm.SCORE)
                    if res is not None:
                        self.memo[key] = res["score"]
                        yield res["score"]
                yield None


def main():
    # TODO:
    # 3. After evolution print best info.
    # 7. Old ZMQ version from other branch (guards check)
    args, unknown = optimizer_arguments().parse_known_args()

    if args.local_workers:  # Local mode.
        # Check sanity.
        if ("-coil" not in unknown or "-grid" not in unknown
            or "-fine" not in unknown):
            printf("You probably missed some important arguments."
                "Evaluation process(es) is unlikely to start."
                "Check for -coil, -grid, -fine arguments."
                "Exiting.")
            sys.exit(0)

        printf("Starting optimization with local workers (%d)" %
            args.local_workers)

        # Prepare the ZeroMQ communication layer.
        cc = LocalClientComm()

        # Arguments to be passed to evaluator processes.
        evaluator_args = unknown + ["-local-mode", "-local-pull-address",
            cc.push_addr, "-local-publish-address", cc.sub_addr]

        workers = []
        # Launch desired number of worker processes.
        if args.evaluator:
            command = args.evaluator
        else:
            this_dir = os.path.dirname(os.path.abspath(__file__))
            command = os.path.join(this_dir, "evaluator_block.py")

        for i in range(args.local_workers):
            p = subprocess.Popen([command] + evaluator_args,
                stdout=subprocess.PIPE, stdin=subprocess.PIPE)
            workers.append(p)

        # Wait until (presumably) all workers are awake and ready
        # to avoid unfair distribution of tasks.
        time.sleep(1)

        # Kill children workers at exit.
        @atexit.register
        def stop_workers():
            for proc in workers:
                proc.kill()

    else:  # Network mode.
        printf("Starting optimization with network workers")
        if not args.remote_workers:
            raise RuntimeError("You have to specify a list of remote"
                " worker addresses")

        # Parse workers addresses.
        hosts = args.remote_workers.split(",")
        workers = []
        for host in hosts:
            parts = host.split(":")
            if len(parts) == 3:  # Full format.
                workers.append(tuple(parts))
            else:  # Hostname only or wrong format.
                workers.append((parts[0], "5558", "5559"))

        # Connect to workers with ZeroMQ.
        cc = NetworkClientComm(addresses=workers)
        # Wait until we establish a connection with remote workers.
        time.sleep(2)

    printf("Waiting for initial connection")

    # Fetch constraints from any worker.
    cc.request("", 0, cc.QUERY_CONSTRAINTS)
    resp = cc.response_wait(0, cc.RESP_CONSTRAINTS)
    no_vars = resp["no_vars"]
    my_min = resp["min_constr"]
    my_max = resp["max_constr"]
    printf("Received initial data from workers")

    # Prepare the GA engine.
    # Initialize genome with constraints.
    if args.allele:  # Built-in allele version.
        genome = AlleleG1DList(no_vars, constr_min=my_min, constr_max=my_max)
    else:  # Custom, ported genetic operators.
        genome = CustomG1DList(no_vars)
        genome.setParams(min_constr=my_min, max_constr=my_max)
    genome.evaluator.set(MemoizedObjective(cc).objective)
    # Set GA engine parameters.
    ga = CustomGSimpleGA(genome, args.seed)
    ga.setPopulationSize(args.popsize or 200)
    ga.setGenerations(args.ngen or 100)
    ga.setParams(stop_file=args.stopflag)

    # Fire the Genetic Algorithm Engine.
    printf("Starting GA: %d generations of %d individuals" % (ga.nGenerations,
        ga.getPopulation().popSize))
    timer = Timer().start()
    ga.setParams(timer=timer)
    ga.evolve()
    stats_step_callback(ga)  # Display final statistics.

    # Evolution is stoped.
    run_time = timer.stop()
    print "GA finished in %g s." % run_time

    ## TODO: End of evaluation output and files. ##
    # bridge.coil_to_print(ga.bestIndividual().getInternalList())

    # TODO: We don't know if user specified paths
    # Save optimization results CBlock output.
    data = {"params": ga.bestIndividual().getInternalList()}
    cc.request(data, 1, cc.SAVE_CBLOCK)
    resp = cc.response_wait(1, cc.CBLOCK_SAVED)
    if resp["status"] == 0:
        print "Saved CBlock file."

    # XML output.
    data = {"params": ga.bestIndividual().getInternalList()}
    cc.request(data, 2, cc.SAVE_XML)
    resp = cc.response_wait(2, cc.XML_SAVED)
    if resp["status"] == 0:
        print "Saved XML file."

    # Close communicator.
    cc.close()


if __name__ == '__main__':
    main()
