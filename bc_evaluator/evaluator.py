from common.utils import libs_to_path
libs_to_path()  # Insert path to libs directory.
# TODO: Handle each dependency separately

import pyximport
pyximport.install()

import bridge
from common.communication import (LocalServerComm, NetworkServerComm,
    BaseHandler)


class BlockCoilEvaluator(BaseHandler):
    def __init__(self, args):
        self.args = args
        self.__init_comm()
        self.__init_bridge()

    def __init_comm(self):
        if self.args.local_mode:
            # Prepare the ZeroMQ communication layer.
            print "Starting Evalutor in local mode."
            self.comm = LocalServerComm(addresses=[self.args.local_pull,
                self.args.local_publish])
        else:
            print "Starting Evalutor in network mode."
            self.comm = NetworkServerComm(ports=[self.args.network_pull,
                self.args.network_publish])

    def __init_bridge(self):
        # Prepare the C++ layer of optimizer through the Cython bridge.
        bridge.prepare(self.args)
        # Pull optimization constraints from C++ layer.
        options = bridge.get_optimization_params()
        self.no_vars, self.min_constr, self.max_constr = options

    def handle_get_options(self, data, s_id):
        self.comm.resp_options({
                "num_params": self.no_vars,
                "min_constr": self.min_constr,
                "max_constr": self.max_constr,
            }, s_id)

    def handle_evaluate(self, data, s_id):
        score = bridge.bfun(data["params"])
        self.comm.resp_score({"score": score}, s_id)

    def handle_get_stats(self, data, s_id):
        self.comm.resp_stats({"stats": "TODO: Not implemented"}, s_id)

    def handle_save_output(self, data, s_id):
        params = data["params"]
        saved = []
        if self.args.outcb:
            bridge.save_cblock(self.args.outcb, params)
            saved.append(self.args.outcb)
        if self.args.outxml:
            bridge.save_xml(self.args.outxml, params, self.args.density)
            saved.append(self.args.outxml)
        self.comm.resp_save({
                "status": "",
                "files": saved,
            }, s_id)

    def run(self):
        self.comm.listen_forever(self)


def main(args):
    BlockCoilEvaluator(args).run()


__all__ = ["main"]
