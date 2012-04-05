import zmq
from base import BaseServerComm


class LocalServerComm(BaseServerComm):
    def __init__(self, addresses, context=None):
        super(LocalServerComm, self).__init__(context)

        self.listener = self.ctx.socket(zmq.PULL)
        self.listener.connect(addresses[0])

        self.publisher = self.ctx.socket(zmq.PUB)
        self.publisher.connect(addresses[1])


class NetworkServerComm(BaseServerComm):
    def __init__(self, ports, context=None):
        super(NetworkServerComm, self).__init__(context)

        self.listener = self.ctx.socket(zmq.PULL)
        self.listener.bind("tcp://*:%s" % ports[0])

        self.publisher = self.ctx.socket(zmq.PUB)
        self.publisher.bind("tcp://*:%s" % ports[1])


__all__ = ["LocalServerComm", "NetworkServerComm"]
