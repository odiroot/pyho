#!/usr/bin/env python
import argparse
from optimizer import HybridOptimizer


def optimizer_arguments():
    parser = argparse.ArgumentParser()
    parser.description = u"Genetic Algorithm optimizer for coil design."
    parser.epilog = u"""This application is a part of PyHO package."""

    # Run settings.
    parser.add_argument("-stopflag", metavar="<filename>", dest="stopflag",
        type=str, help=u"Path to the stop-signalling file")
    parser.add_argument("-evaluator-path", metavar="<path>", dest="evaluator",
        type=str, help=u"Used only in local mode. Path to the evaluator"
            " launcher. If not specified block coil evaluator is assumed.")

    workers = parser.add_mutually_exclusive_group()
    workers.add_argument("-local-workers", metavar="<number>",
        dest="local_workers", type=int,
        help=u"Activates local computation mode with a specified number of"
            " workers. Uses IPC communication.")
    workers.add_argument("-remote-workers", metavar="<worker adresses>",
        dest="remote_workers", type=str, help=u"""
            A list of worker processes listening addresses e.g.:
            "host1,host2,host3" or
            "host1:pull_port:publish_port,host2:pull_port:publish_port"
            Pull port is used to receive tasks from manager process, it
            defaults to 5558.
            Publish port is used to send results back to manager process, it
            defaults to 5559.""")

    # Genetic Algorithm parameters.
    genetic = parser.add_argument_group("Genetic optimization")
    genetic.add_argument("-seed", metavar="<value>", dest="seed",
        type=int, help=u"Start the evolution with a specified random seed")
    genetic.add_argument("-ngen", metavar="<value>", dest="ngen",
        type=int, help=u"Number of GA generations to run")
    genetic.add_argument("-popsize", metavar="<value>", dest="popsize",
        type=int, help=u"Size of the GA population")
    genetic.add_argument("-allele", action="store_true", dest="allele",
        help=u"Whether to use Allele operators", default=False)

    # Levmar parameters.
    levmar = parser.add_argument_group("Levenberg-Marquardt optimization")
    levmar.add_argument("-iter", metavar="<value>", dest="iter", type=int,
        help=u"Number of levmar algorithm iterations to run")
    levmar.add_argument("-central-difs", dest="central", action="store_true",
        help=u"Turns of central differences mode in Levmar.")

    return parser


def main():
    args, unknown = optimizer_arguments().parse_known_args()

    local = args.local_workers is not None
    optimizer = HybridOptimizer(evaluator_path=args.evaluator, local=local, 
        local_workers=args.local_workers, remote_workers=args.remote_workers,
        unknown_args=unknown, stop_flag=args.stopflag, ga_seed=args.seed,
        ga_iter=args.ngen, ga_size=args.popsize, ga_allele=args.allele,
        lm_iter=args.iter, lm_central=args.central)
    optimizer.run()


if __name__ == '__main__':
    main()

