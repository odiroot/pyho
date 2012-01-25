#!/usr/bin/env python
import os
import sys
import tempfile
import subprocess
import atexit

# Insert path to libs directory.
currdir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(currdir, os.path.pardir, "libs"))

from utils import optimizer_arguments, Timer
from communication import ClientComm
from ga_common import CustomG1DList, CustomGSimpleGA, stats_step_callback
from ga_common import AlleleG1DList


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
    # 3. After evolution print gest best info.
    # 4. Saving output files.
    # 5. Output messages transport via PUB/SUB.
    args, unknown = optimizer_arguments().parse_known_args()

    if args.workers:  # Local mode.
        # Generate temporary paths to avoid collisions.
        fn, push_ipc = tempfile.mkstemp("pyho_push")
        os.close(fn)
        fn, sub_ipc = tempfile.mkstemp("pyho_sub")
        os.close(fn)

        push_addr = "ipc://%s" % push_ipc
        sub_addr = "ipc://%s" % sub_ipc

        # Arguments to be passed to evaluator processes.
        evaluator_args = unknown + ["-local", "-pull-address", push_ipc,
            "-publish-address", sub_ipc]

        workers = []
        # Launch desired number of worker processes.
        for i in range(args.workers or 1):
            # TODO: Changing evaluator path.
            command = ["./evaluator_block.py"] + evaluator_args
            p = subprocess.Popen(command, stdout=subprocess.PIPE,
                stdin=subprocess.PIPE, stderr=subprocess.PIPE)
            workers.append(p)

        # Kill children workers at exit.
        @atexit.register
        def stop_workers():
            for proc in workers:
                proc.kill()

    else:  # Network mode.
        push_addr = "tcp://*:%d" % args.push
        sub_addr = "tcp://*:%d" % args.subscribe

    # Prepare the ZeroMQ communication layer.
    cc = ClientComm(push_addr=push_addr, sub_addr=sub_addr)

    # Fetch constraints from any worker.
    cc.request("", 0, cc.QUERY_CONSTRAINTS)
    resp = cc.response_wait(0, cc.RESP_CONSTRAINTS)
    no_vars = resp["no_vars"]
    my_min = resp["min_constr"]
    my_max = resp["max_constr"]

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
    print "Starting GA: %d generations of %d individuals" % (ga.nGenerations,
        ga.getPopulation().popSize)
    timer = Timer().start()
    ga.setParams(timer=timer)
    ga.evolve()
    stats_step_callback(ga)  # Display final statistics.

    # Evolution is stoped.
    run_time = timer.stop()
    print "GA finished in %g s." % run_time

    ## TODO: End of evaluation output and files. ##
    # bridge.coil_to_print(ga.bestIndividual().getInternalList())
    # # CBlock output.
    # if args.outcb:
    #     bridge.save_cblock(args.outcb)
    # # XML output.
    # if args.outxml:
    #     bridge.save_xml(args.outxml, ga.bestIndividual().getInternalList(),
    #         args.density)

    # Cleaning state.
    # Broadcast exit messages.
    if args.send_exit:
        for i in range(10):
            cc.request("", 0, cc.EXIT_SIGNAL)
    # Close communicator.
    cc.close()


if __name__ == '__main__':
    main()
