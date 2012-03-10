import os
import time
import tempfile
import zmq


class MessageTypeError(ValueError):
    message = "Wrong response type from the transport."


class MessageType(object):
    QUERY_CONSTRAINTS = 1
    RESP_CONSTRAINTS = 2
    DO_EVALUATION = 10
    SCORE = 11
    SAVE_CBLOCK = 20
    CBLOCK_SAVED = 21
    SAVE_XML = 30
    XML_SAVED = 31

    EXIT_SIGNAL = 99


class BaseServerComm(MessageType):
    handlers = {}

    def __init__(self, context=None):
        self.ctx = context or zmq.Context()

        self.listener = None
        self.publisher = None

    def receive(self):
        return self.listener.recv_json()

    def send(self, msg, s_id, m_type):
        self.publisher.send_json({
            "data": msg,
            "id": s_id,
            "type": m_type,
        })

    def all_handle(self):
        resp = self.receive()
        m_type = resp["type"]
        if m_type in self.handlers:
            handler = self.handlers[m_type]
            return handler(resp["data"], resp["id"], self)
        else:
            raise MessageTypeError

    def __setitem__(self, m_type, func):
        self.handlers[m_type] = func


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


class BaseClientComm(MessageType):
    store = {}
    POLL_INTERVAL = 1 / 1000

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

    def request(self, msg, s_id, m_type=None):
        self.sender.send_json({
            "data": msg,
            "id": s_id,
            "type": m_type,
        })

    def response(self, s_id, m_type=None):
        if s_id in self.store:
            resp = self.store.pop(s_id)
            if resp["type"] != m_type:
                raise MessageTypeError
            return resp["data"]
        else:
            has_data = bool(self.receiver.getsockopt(zmq.EVENTS) & zmq.POLLIN)
            if has_data:
                resp = self.receiver.recv_json()
                if resp["id"] == s_id:
                    if resp["type"] != m_type:
                        raise MessageTypeError
                    return resp["data"]
                else:
                    self.store[resp["id"]] = resp
                    return None
            else:
                return None

    def response_wait(self, s_id, m_type=None, sleep=POLL_INTERVAL):
        while True:
            resp = self.response(s_id, m_type)
            if resp is not None:
                return resp
            time.sleep(sleep)


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
