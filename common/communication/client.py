import time
import zmq
import os
import tempfile
from base import MessageType, MessageTypeError


class BaseClientComm(MessageType):
    store = {}

    def __init__(self, context=None):
        self.ctx = context or zmq.Context()

        self.sender = None
        self.receiver = None

    def close(self, linger=1000):
        self.sender.setsockopt(zmq.LINGER, linger)
        self.receiver.setsockopt(zmq.LINGER, linger)
        self.sender.close()
        self.receiver.close()
        if self.store:
            print "Warning: client communicator store not empty."

    def send_request(self, msg, s_id, m_type, wait=True):
        payload = {"data": msg, "id": s_id, "type": m_type}
        if wait:
            self.sender.send_json(payload)
        else:
            poller = zmq.Poller()
            poller.register(self.sender, zmq.POLLOUT)
            if poller.poll(100):
                self.sender.send_json(payload)
                return True
            else:
                return False

    def __validate(self, resp, m_type):
        if resp["type"] != m_type:
            raise MessageTypeError
        return resp["data"]

    def __get_non_blocking(self, s_id, m_type):
        if s_id in self.store:
            return self.__validate(self.store.pop(s_id), m_type)
        else:
            has_data = bool(self.receiver.getsockopt(zmq.EVENTS) & zmq.POLLIN)
            if has_data:
                resp = self.receiver.recv_json()
                if resp["id"] == s_id:
                    return self.__validate(resp, m_type)
                else:
                    self.store[resp["id"]] = resp
            return None

    def get_response(self, s_id, m_type, wait=False):
        while True:
            resp = self.__get_non_blocking(s_id, m_type)
            if resp is None and wait:
                time.sleep(1 / 1000.)
            else:
                return resp


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
