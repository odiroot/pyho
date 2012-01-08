u"""Various utilities for PyHO."""
import argparse
import sys
import time


class MessageTypeError(ValueError):
    message = "Wrong response type from the transport."


class MessageType(object):
    QUERY_CONSTRAINTS = 1
    RESP_CONSTRAINTS = 2
    DO_EVALUATION = 10
    SCORE = 11

    EXIT_SIGNAL = 99


def optimizer_arguments():
    parser = argparse.ArgumentParser()
    parser.description = u"Genetic Algorithm optimizer for coil design."
    parser.epilog = u"""This application is a part of PyHO package."""

    # Run settings.
    parser.add_argument("-log", metavar="<logfile>", dest="logfile",
        type=str, help=u"log file path")
    parser.add_argument("-stopflag", metavar="<filename>", dest="stopflag",
        type=str, help=u"path to the stop-signalling file")
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
    parser.add_argument("-log", metavar="<logfile>", dest="logfile",
        type=str, help=u"log file path")
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


def color_print(text, bold=False, color='32', target=None):
    if bold:
        color = "1;%s" % color
    text = "\033[%sm%s\033[0m" % (color, text)
    print >> target or sys.stdout, text


def error_print(text):
    color_print(text, color="31", target=sys.stderr)


def exit_error(text):
    "Stop program execution and print error string to stderr"
    text = "\n" + text + "\nExiting abnormally\n"
    error_print(text)
    sys.exit(-1)


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
