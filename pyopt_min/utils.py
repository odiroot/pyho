u"""Various utilities for PyHO."""
import argparse
import sys
import time


def optimizer_arguments():
    parser = argparse.ArgumentParser()
    parser.description = u"Genetic Algorithm optimizer for coil design."
    parser.epilog = u"""This application is a part of PyHO package."""

    # Run settings.
    parser.add_argument("-log", metavar="<logfile>", dest="logfile",
        type=str, help=u"log file path")
    parser.add_argument("-stopflag", metavar="<filename>", dest="stopflag",
        type=str, help=u"path to the stop-signalling file")

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
    parser.add_argument("-send-exit", action="store_true", default=False,
        help=u"Whether to kill remote workers on optimizer exit")

    # Genetic Algorithm parameters.
    parser.add_argument("-seed", metavar="<value>", dest="seed",
        type=int, help=u"start the evolution with a specified random seed")
    parser.add_argument("-ngen", metavar="<value>", dest="ngen",
        type=int, help=u"number of GA generations to run")
    parser.add_argument("-popsize", metavar="<value>", dest="popsize",
        type=int, help=u"size of the GA population")
    parser.add_argument("-allele", metavar="<switch>", dest="allele",
        type=bool, help=u"Whether to use Allele operators", default=False)
    # Not much use right now.
    #parser.add_argument("-algorithm", dest="algorithm",
    #    choices=["GASimpleGA"], help=u"genetic algorithm engine")
    return parser


def evaluator_arguments():
    parser = argparse.ArgumentParser()
    parser.description = u"Coil evaluator for optimal coil design."
    parser.epilog = u"""This application is a part of PyHO package."""

    # Run settings
    parser.add_argument("-log", metavar="<logfile>", dest="logfile",
        type=str, help=u"log file path")

    # Local mode.
    parser.add_argument("-local-mode", action="store_true", default=False,
        help=u"Work as a local worker communicating through IPC."
        " Should be launched automatically by the optimizer process.")
    local = parser.add_argument_group("Local mode")
    local.add_argument("-local-pull-address", metavar="<address>",
        dest="local_pull", type=str, help=u"Listening address of task manager")
    local.add_argument("-local-publish-address", metavar="<address>",
        dest="local_publish", type=str, help=u"Task result publishing address")

    # Input files.
    parser.add_argument("-coil", metavar="<param-coil-file>", dest="coil",
        required=True, type=str, help=u"model coil definition")
    parser.add_argument("-grid", metavar="<grid-file>", dest="grid",
        required=True, type=str, help=u"grid for optimization")
    # Output files.
    parser.add_argument("-xml", metavar="<out-xml-file>", dest="outxml",
        type=str, help=u"output of the optimization process")
    parser.add_argument("-cblock", metavar="<out-cblock-file>", dest="outcb",
        type=str, help=u"???")
    # Model parameters.
    parser.add_argument("-fine", metavar="<density>", dest="density",
        required=True, type=int, help=u"grid density for final evaluation")
    parser.add_argument("-Bx", metavar="<value>", dest="Bx",
        type=float, help=u"set desired absolute value of "
        "the Bx component")
    parser.add_argument("-By", metavar="<value>", dest="By",
        type=float, help=u"set desired absolute value of "
        "the By component")
    parser.add_argument("-Bz", metavar="<value>", dest="Bz",
        type=float, help=u"set desired absolute value of "
        "the Bz component")
    # Not used right now.
    # parser.add_argument("-Bfile", metavar="<out-field-file>", dest="outfield",
    #     #required=True,
    #     type=str, help=u"???")
    # parser.add_argument("-statistics", dest="statistics", action="store_true",
    #     help=u"???")
    return parser


class Timer(object):
    u"A simple timer utility with lap feature"
    def __init__(self):
        self.t1 = 0
        self.t2 = 0
        self.t1lap = 0
        self.time_so_far = 0

    def start(self):
        self.t1 = time.time()
        self.t1lap = time.time()
        return self

    def stop(self):
        self.t2 = time.time()
        return self.t2 - self.t1

    def lap(self):
        self.t2 = time.time()
        tmp = self.t2 - self.t1lap
        self.t1lap = self.t2
        self.time_so_far += tmp
        return tmp


class RedirecredWriter(object):
    u"An utility to pass (and redirect) all stdout prints to a function."
    def __init__(self, print_func):
        self._old_stdout = sys.stdout
        self.print_func = print_func
        sys.stdout = self

    def __del__(self):
        sys.stdout = self._old_stdout

    def write(self, *args):
        text = args[0]
        if text != '\n':
            self.print_func(text)
