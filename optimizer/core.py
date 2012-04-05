import os
import subprocess
import atexit
import sys
import time

# Insert path to libs directory.
from common.utils import libs_to_path
libs_to_path()

from common.utils import Timer, printf, check_stop_flag
from common.communication import LocalClientComm, NetworkClientComm
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
                    self.comm.evaluate(p, uid)
                    sent = True
                if sent:
                    # Try to retrieve evaluation score from the transport.
                    res = self.comm.resp_score(uid)
                    if res is not None:
                        self.memo[key] = res["score"]
                        yield res["score"]
                yield None


def default_evaluator_path():
    this_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(this_dir, os.path.pardir,
        "run_bc_evaluator"))


def main(args, unknown):
    # TODO:
    # 3. After evolution print best info.
    if args.local_workers:  # Local mode.
        # Check sanity.
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
            command = default_evaluator_path()

        for i in range(args.local_workers):
            p = subprocess.Popen([command] + evaluator_args,
                stdout=subprocess.PIPE, stdin=subprocess.PIPE,
                #stderr=subprocess.PIPE
                )
            workers.append(p)

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

    # Wait until (presumably) all workers are awake and ready
    # to avoid unfair distribution of tasks.
    time.sleep(1)
    printf("Waiting for initial connection")

    # Fetch constraints from any worker.
    _sent = False
    while not _sent:
        _sent = cc.get_options(0, wait=False)
        if check_stop_flag(args.stopflag):
            sys.exit(-1)
    resp = cc.resp_options(0, wait=True)

    no_vars = resp["num_params"]
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

    # Save optimization results CBlock output.
    cc.save_output(ga.bestIndividual().getInternalList(), 1)
    resp = cc.resp_save(1, wait=True)
    if resp["status"] == "":
        print "Saved files: %s" % ', '.join(resp["files"])

    # Close communicator.
    cc.close()


__all__ = ["main"]
