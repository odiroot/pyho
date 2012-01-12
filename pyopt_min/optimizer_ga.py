#!/usr/bin/env python
import os
import sys
import zmq

# Insert path to libs directory.
currdir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(currdir, os.path.pardir, "libs"))

from utils import optimizer_arguments, Timer, ClientComm
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
        uid = id(chromosome)  # Unique id for messaging.

        sent = False  # Whether evaluation request is sent.
        # We work in a generator to hold some state.
        while True:
            if not sent:
                # Send asynchronous request for evaluation.
                self.comm.request({"params": p}, uid, self.comm.DO_EVALUATION)
                sent = True
            if sent:
                # Try to retrieve evaluation score from the transport.
                res = self.comm.response(uid, self.comm.SCORE)
                yield res["score"] if res else None
            yield None


def main():
    # TODO:
    # 3. After evolution print gest best info.
    # 4. Saving output files.
    # 5. Output messages transport via PUB/SUB.
    args = optimizer_arguments().parse_args()

    # Prepare the ZeroMQ communication layer.
    ctx = zmq.Context()
    cc = ClientComm(args.workers, ctx)

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
    # bridge.print_best(ga.bestIndividual().getInternalList())

    # # CBlock output.
    # if args.outcb:
    #     bridge.save_cblock(args.outcb)
    # # Rebuild grid with new density if fine is specified.
    # if args.density:
    #     bridge.rebuild(args.density)
    # # XML output.
    # if args.outxml:
    #     bridge.save_xml(args.outxml, ga.bestIndividual().getInternalList())

    # TODO: Bfile.
    print cc.store

if __name__ == '__main__':
    main()
