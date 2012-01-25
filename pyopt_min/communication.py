import zmq
import time


class MessageTypeError(ValueError):
    message = "Wrong response type from the transport."


class MessageType(object):
    QUERY_CONSTRAINTS = 1
    RESP_CONSTRAINTS = 2
    DO_EVALUATION = 10
    SCORE = 11

    EXIT_SIGNAL = 99


class ServerComm(MessageType):
    handlers = {}

    def __init__(self, pull_addr, pub_addr, context=None):
        self.ctx = context or zmq.Context()

        self.listener = self.ctx.socket(zmq.PULL)
        self.listener.connect(pull_addr)

        self.publisher = self.ctx.socket(zmq.PUB)
        self.publisher.connect(pub_addr)

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


class ClientComm(MessageType):
    store = {}
    POLL_INTERVAL = 1 / 1000

    def __init__(self, push_addr, sub_addr, context=None):
        self.ctx = context or zmq.Context()

        self.sender = self.ctx.socket(zmq.PUSH)
        self.sender.bind(push_addr)

        self.receiver = self.ctx.socket(zmq.SUB)
        self.receiver.setsockopt(zmq.SUBSCRIBE, '')
        self.receiver.bind(sub_addr)
        time.sleep(0.5)  # Wait for socket synchronization.

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
