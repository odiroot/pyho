#!/usr/bin/env python
import os
import sys

# Insert path to libs directory.
currdir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(currdir, os.path.pardir, "libs"))
import pyximport
pyximport.install()
from bridge_block import bridge

from utils import evaluator_arguments
from communication import LocalServerComm, NetworkServerComm


def main():
    args = evaluator_arguments().parse_args()

    if args.local_mode:
        # Prepare the ZeroMQ communication layer.
        print "Starting Evalutor in local mode."
        sc = LocalServerComm(addresses=[args.local_pull, args.local_publish])
    else:
        print "Starting Evalutor in network mode."
        sc = NetworkServerComm(ports=[args.network_pull, args.network_publish])

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
        params = data["params"]
        score = bridge.bfun(params)
        resp = {"score": score}
        comm.send(resp, s_id, comm.SCORE)

    def handle_exit(data, s_id, comm):
        print "Exiting due to a remote command..."
        sys.exit(0)

    def handle_save_cblock(data, s_id, comm):
        params = data["params"]
        if args.outcb:
            bridge.save_cblock(args.outcb, params)
            comm.send({"status": 0}, s_id, comm.CBLOCK_SAVED)
        else:
            comm.send({"status": -1}, s_id, comm.CBLOCK_SAVED)

    def handle_save_xml(data, s_id, comm):
        params = data["params"]
        if args.outxml:
            bridge.save_xml(args.outxml, params, args.density)
            comm.send({"status": 0}, s_id, comm.XML_SAVED)
        else:
            comm.send({"status": -1}, s_id, comm.XML_SAVED)

    # Assign handlers.
    sc[sc.QUERY_CONSTRAINTS] = handle_constraints
    sc[sc.DO_EVALUATION] = handle_evaluation
    sc[sc.EXIT_SIGNAL] = handle_exit
    sc[sc.SAVE_CBLOCK] = handle_save_cblock
    sc[sc.SAVE_XML] = handle_save_xml

    while True:  # The famous Main Loop.
        # Wait for the request and handle it.
        sc.all_handle()


if __name__ == '__main__':
    main()
