u"""
    An implementation of basic evaluator using Rosenbrock function.
    This example doesn't depend on PyHO libraries -- it's meant to
    be a standalone testing tool.
"""
import zmq


# A ZeroMQ global context.
ctx = zmq.Context()


# PyHO protocol: message types.
class MessageType(object):
    GET_OPTIONS = 1
    RESP_OPTIONS = 101
    EVALUATE = 2
    RESP_SCORE = 102
    GET_STATS = 3
    RESP_STATS = 103
    SAVE_OUTPUT = 4
    RESP_SAVE = 104


class RosenbrockEvaluator(object):
    NUM_PARAMS = 2
    # Default constraints for params.
    CONSTRAINTS = [
        [-100., 100.],
        [-100., 100.],
    ]

    def __init__(self, local_mode=False, addresses=None, ports=None):
        # Set up communication.
        # ZMQ socket listening for commands.
        self.listener = ctx.socket(zmq.PULL)
        # ZMQ socket publishing results.
        self.publisher = ctx.socket(zmq.PUB)
        # Depending on the mode connect to a node or start listening.
        print "Starting Rosenbrock evalutor",
        if local_mode:
            print "in local mode."
            self.listener.connect(addresses[0])
            self.publisher.connect(addresses[1])
        else:
            print "in network mode."
            self.listener.bind("tcp://*:%s" % ports[0])
            self.publisher.bind("tcp://*:%s" % ports[1])

    def run(self):
        print "Started"
