import os
import tempfile
import zmq
from base import BaseClientComm


class LocalClientComm(BaseClientComm):
    def __init__(self, addresses=None, context=None):
        super(LocalClientComm, self).__init__(context)

        if addresses:  # If not specified generate temporary sockets.
            self.push_addr, self.sub_addr = addresses
        else:
            self.push_addr, self.sub_addr = self._prepare_addresses()

        self.sender = self.ctx.socket(zmq.PUSH)
        self.sender.bind(self.push_addr)

        self.receiver = self.ctx.socket(zmq.SUB)
        self.receiver.setsockopt(zmq.SUBSCRIBE, '')
        self.receiver.bind(self.sub_addr)

    def _prepare_addresses(self):
        # TODO: Handle Windows not supporting IPC.
        # Generate temporary paths to avoid collisions.
        fn, push_ipc = tempfile.mkstemp("pyho_push")
        os.close(fn)
        fn, sub_ipc = tempfile.mkstemp("pyho_sub")
        os.close(fn)
        push_addr = "ipc://%s" % push_ipc
        sub_addr = "ipc://%s" % sub_ipc
        return push_addr, sub_addr


class NetworkClientComm(BaseClientComm):
    def __init__(self, addresses=None, context=None):
        super(NetworkClientComm, self).__init__(context)

        self.sender = self.ctx.socket(zmq.PUSH)
        self.receiver = self.ctx.socket(zmq.SUB)
        self.receiver.setsockopt(zmq.SUBSCRIBE, '')

        for node in addresses:
            host = node[0]
            self.sender.connect("tcp://%s:%s" % (host, node[1]))
            self.receiver.connect("tcp://%s:%s" % (host, node[2]))


__all__ = ["LocalClientComm", "NetworkClientComm"]
