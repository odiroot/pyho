# Communication classes from PyHO.
from pyho.common.communication import LocalServerComm, NetworkServerComm
# Skeleton class for evaluator.
from pyho.common.communication import BaseHandler


class RosenbrockEvaluator(BaseHandler):
    def __init__(self, local_mode=False, addresses=None, ports=None):
        # Initialize communication,.
        if local_mode:
            print "Starting Evalutor in local mode."
            self.comm = LocalServerComm(addresses=addresses)
        else:
            print "Starting Evalutor in network mode."
            self.comm = NetworkServerComm(ports=ports)

    def run(self):
        self.comm.listen_forever(handler=self)
