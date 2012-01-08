#!/usr/bin/env python
from datetime import datetime
import os
import sys
import zmq

# Insert path to libs directory.
currdir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(currdir, os.path.pardir, "libs"))
import pyximport
pyximport.install()
from bridge import bridge

from utils import evaluator_arguments, MessageType


def main():
    args = evaluator_arguments().parse_args()

    # Prepare the ZeroMQ communication layer.
    ctx = zmq.Context()
    socket = ctx.socket(zmq.REP)
    socket.bind("tcp://*:%d" % args.port)

    # Prepare the C++ layer of optimizer through the Cython bridge.
    bridge.prepare(args)
    # Pull optimization constraints from C++ layer.
    no_vars, min_constr, max_constr = bridge.get_optimization_params()

    while True:  # The famous Main Loop.
        # Wait for the request, we *assume* JSON format.
        message = socket.recv_json()
        msgtype = message["type"]

        if msgtype == MessageType.QUERY_CONSTRAINTS:
            socket.send_json({
                "type": MessageType.RESP_CONSTRAINTS,
                "no_vars": no_vars,
                "min_constr": min_constr,
                "max_constr": max_constr,
            })

        elif msgtype == MessageType.DO_EVALUATION:
            print "%s: Doing an evaluation" % datetime.now()
            params = message["params"]
            score = bridge.bfun(params)
            socket.send_json({
                "type": MessageType.SCORE,
                "score": score,
            })

        elif msgtype == MessageType.EXIT_SIGNAL:
            print "Exiting due to a remote command..."
            sys.exit(0)

        else:
            raise ValueError("Unknown message type")


if __name__ == '__main__':
    main()
