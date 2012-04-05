import time
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
