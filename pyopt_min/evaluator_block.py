#!/usr/bin/env python
from datetime import datetime
import time
import os
import sys
import zmq

# Insert path to libs directory.
currdir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(currdir, os.path.pardir, "libs"))
import pyximport
pyximport.install()
from bridge import bridge

from utils import evaluator_arguments, ServerComm


def main():
    args = evaluator_arguments().parse_args()

    # Prepare the ZeroMQ communication layer.
    ctx = zmq.Context()
    sc = ServerComm("*:%d" % args.port, ctx)

    # Prepare the C++ layer of optimizer through the Cython bridge.
    bridge.prepare(args)
    # Pull optimization constraints from C++ layer.
    no_vars, min_constr, max_constr = bridge.get_optimization_params()

    # Define request handlers.
    def handle_constraints(data, s_id, comm):
        resp = {
            "no_vars": no_vars,
            "min_constr": min_constr,
            "max_constr": max_constr,
        }
        comm.send(resp, s_id, comm.RESP_CONSTRAINTS)

    def handle_evaluation(data, s_id, comm):
        print s_id
        params = data["params"]
        score = bridge.bfun(params)
        resp = {"score": score}
        comm.send(resp, s_id, comm.SCORE)

    def handle_exit(data, s_id, comm):
        print "Exiting due to a remote command..."
        sys.exit(0)

    # Assign handlers.
    sc[sc.QUERY_CONSTRAINTS] = handle_constraints
    sc[sc.DO_EVALUATION] = handle_evaluation
    sc[sc.EXIT_SIGNAL] = handle_exit

    while True:  # The famous Main Loop.
        # Wait for the request and handle it.
        sc.all_handle()


if __name__ == '__main__':
    main()
