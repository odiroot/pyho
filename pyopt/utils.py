u"""Various utilities for PyHO."""
import argparse
import sys


def cli_arguments():
    u"Command line argument parser for PyHO options."
    apars = argparse.ArgumentParser()
    apars.description = u"Genetic Algorithm optimizer for coil design."
    apars.epilog = u"""
        This application is a part of PyHO package.        
        XXX ATTRIBUTION DISCLAIMER
    """
    #apars.prog = u"optimizer_ga.py"
    
    apars.add_argument("-log", metavar="<logfile>", dest="logfile",
        type=str, help=u"log file path")
    apars.add_argument("-stopflag", metavar="<filename>", dest="stopflag",
        type=str, help=u"path to the stop-signalling file")
    # Genetic Algorithm parameters.
    apars.add_argument("-seed", metavar="<value>", dest="seed",
        type=int, help=u"start the evolution with a specified random seed")
    #apars.add_argument("-algorithm", dest="algorithm",
    #    choices=["GASimpleGA"], help=u"genetic algorithm engine")    
    apars.add_argument("-ngen", metavar="<value>", dest="ngen",
        type=int, help=u"number of GA generations to run")
    apars.add_argument("-popsize", metavar="<value>", dest="popsize",
        type=int, help=u"size of the GA population")    
    return apars


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

__all__ = ["cli_arguments", "error_print", "exit_error"]
