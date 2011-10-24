#!/usr/bin/env python
from pyevolve.G1DList import G1DList
from pyevolve.GSimpleGA import GSimpleGA
from pyevolve.Initializators import G1DListInitializatorReal
from pyevolve.Consts import minimaxType
from utils import cli_arguments


def objective(chromosome):
    raw_data = chromosome.getInternalList()
    return bridge.bfun(raw_data)


def main():
    u"`optimizer_ga` logic"
    # Parsing command line arguments.
    args, unknown = cli_arguments().parse_known_args()
    eval_args = ' '.join(unknown)
    

    #import zmq
    #addr = "ipc:///tmp/eval"
    #context = zmq.Context()
    #socket = context.socket(zmq.REQ)
    #socket.connect(addr)

    #packed_args = dict(args._get_kwargs())
    #socket.send_json(packed_args)
    # TODO: Start daemons

    # Prepare the C++ layer of optimizer through the Cython bridge.
    #bridge.prepare(args)
    # Pull optimization constraints from C++ layer.
    #no_vars, my_min, my_max = bridge.get_optimization_params()

    # Prepare the GA engine.
    #genome = G1DList(size=no_vars)
    # Evaluation function.
    #genome.evaluator.set(objective, 1.)
    # Initialize genome.
    # TODO: CHANGE TO ALLELE INITIALIZATION (USING MY_MIN, MY_MAX)
    #genome.initializator.set(G1DListInitializatorReal, 1.)
    #genome.setParams(rangemin=min(my_min), rangemax=max(my_max))
    # Set GA engine parameters.
    #ga = GSimpleGA(genome)
    #ga.setMinimax(minimaxType["minimize"])
    #ga.setPopulationSize(50)
    #ga.setGenerations(10)
    #ga.setMutationRate(0.05)
    #ga.setCrossoverRate(1.0)
    # Fire the engines!
    #ga.evolve(freq_stats=1)
    #print ga.bestIndividual()


if __name__ == '__main__':
    import time
    start = time.time()
    main()
