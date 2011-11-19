#!/usr/bin/env python
import os
import sys
import time
# Insert path to libs directory.
currdir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(currdir, os.path.pardir, "libs"))
import pyximport
pyximport.install()

from bridge import bridge
from utils import cli_arguments, Timer, RedirecredWriter
from ga_common import CustomG1DList, CustomGSimpleGA, stats_step_callback

        
def objective(chromosome):
    u"The Genetic Algorithm evaluation function"
    raw_data = chromosome.getInternalList()
    return bridge.bfun(raw_data)


def memoized_objective(obj_fun):
    memo = {}

    def inner_func(chromosome):
        key = tuple(chromosome)
        if key not in memo:
            memo[key] = obj_fun(chromosome)
        return memo[key]
    return inner_func
    

def main():
    u"`optimizer_ga` logic"
    args = cli_arguments().parse_args()  # Parsing command line arguments.
    RedirecredWriter(bridge.cprint)  # Redirecting all stdout prints.

    # Prepare the C++ layer of optimizer through the Cython bridge.
    bridge.prepare(args)
    # Pull optimization constraints from C++ layer.
    no_vars, my_min, my_max = bridge.get_optimization_params()

    # Prepare the GA engine.
    # Initialize genome with constraints.
    genome = CustomG1DList(no_vars)
    genome.setParams(min_constr=my_min, max_constr=my_max)
    genome.evaluator.set(memoized_objective(objective))
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
    bridge.print_best(ga.bestIndividual().getInternalList())

    # CBlock output.
    if args.outcb:
        bridge.save_cblock(args.outcb)  
    # Rebuild grid with new density if fine is specified.
    if args.density:
        bridge.rebuild(args.density)
    # XML output.
    if args.outxml:
        bridge.save_xml(args.outxml, ga.bestIndividual().getInternalList())
    
    # TODO: Bfile.

if __name__ == '__main__':
    start = time.time()
    main()
