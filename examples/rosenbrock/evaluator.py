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


# Rosenbrock function.
def rosenbrock(x, y):
    return (1. - x) ** 2. + 100 * (y - x ** 2.) ** 2.


class RosenbrockEvaluator(object):
    u"""Evaluates two-argument Rosenbrock function over limited domain."""
    NUM_PARAMS = 2
    # Default constraints for params.
    CONSTRAINTS = [
        [-100., -100.],
        [100., 100.],
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
        u"Listen for command and return results."
        while True:  # Run forever.
            # Parse JSON request internally in ZeroMQ.
            request = self.listener.recv_json()
            # Extract command, extra data and session id then handle command.
            self.handle_command(command=request["type"], data=request["data"],
                s_id=request["id"])

    def handle_command(self, command, data, s_id):
        u"Handle request from the optimizer, prepare response."
        handler_map = {
            MessageType.GET_OPTIONS: self.handle_options,
        }

        # Call the handler for a given message type, collect response data.
        try:
            handler = handler_map[command]
        except KeyError:
            raise RuntimeError("Message type uknkown: %s" % command)
        answer, resp_data = handler(data)

        # Reply with adequate response.
        self.send_response(answer, resp_data, s_id)

    def send_response(self, answer, data, s_id):
        u"Form and publish proper JSON response for handled command."
        # Again, let ZeroMQ handle JSON serialization.
        self.publisher.send_json({
            "type": answer,
            "data": data,
            "id": s_id,
        })

    def handle_options(self, *args):
        u"Handle request for optimization options."
        return MessageType.RESP_OPTIONS, {
            "num_params": self.NUM_PARAMS,
            "min_constr": self.CONSTRAINTS[0],
            "max_constr": self.CONSTRAINTS[1],
        }
