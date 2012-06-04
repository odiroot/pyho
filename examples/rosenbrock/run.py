#!/usr/bin/env python
import argparse
from evaluator import RosenbrockEvaluator


def main():
    parser = argparse.ArgumentParser(description="Example Rosenbrock function"
        " evaluator")
    parser.add_argument("-local-mode", action="store_true", default=False)

    local = parser.add_argument_group("Local mode")
    local.add_argument("-local-pull-address", metavar="<address>",
        dest="local_pull", type=str, help=u"Listening address of task manager")
    local.add_argument("-local-publish-address", metavar="<address>",
        dest="local_publish", type=str, help=u"Task result publishing address")

    network = parser.add_argument_group("Network mode")
    network.add_argument("-network-pull-port", metavar="<port>",
        dest="network_pull", type=str, default="5558",
        help=u"Listening port for task manager commands")
    network.add_argument("-network-publish-port", metavar="<port>",
        dest="network_publish", type=str, default="5559",
        help=u"Listening port for task result broadcasting")

    # Rosenbrock params.
    parser.add_argument("-max-x", metavar="<number>", dest="max_x", type=float,
        help=u"Upper limit on X axis.")
    parser.add_argument("-min-x", metavar="<number>", dest="min_x", type=float,
        help=u"Lower limit on X axis.")
    parser.add_argument("-max-y", metavar="<number>", dest="max_y", type=float,
        help=u"Upper limit on Y axis.")
    parser.add_argument("-min-y", metavar="<number>", dest="min_y", type=float,
        help=u"Lower limit on Y axis.")

    # Parse execution arguments.
    args = parser.parse_args()

    # Initialize evaluator and listen for commands.
    e = RosenbrockEvaluator(local_mode=args.local_mode,
        addresses=[args.local_pull, args.local_publish],
        ports=[args.network_pull, args.network_publish],
        max_x=args.max_x, min_x=args.min_x,
        max_y=args.max_y, min_y=args.min_y)
    e.run()


if __name__ == '__main__':
    main()
