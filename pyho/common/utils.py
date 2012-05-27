u"""Various utilities for PyHO."""
import os
import sys
import time


def printf(txt):
    u"Print and flush the stdout"
    print txt
    sys.stdout.flush()


def libs_to_path():
    u"Adds Python path entry pointing to libraries shipped in this project."
    current_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.abspath(os.path.join(current_dir, os.path.pardir))
    libs_dir = os.path.join(base_dir, "libs")
    if libs_dir not in sys.path:
        sys.path.append(libs_dir)


def check_stop_flag(path):
    u"Checks whether user stop flag (file) is present"
    if path:
        return os.path.exists(path)
    else:
        return False


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

