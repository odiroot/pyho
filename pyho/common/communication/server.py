import zmq
from base import MessageType, MessageTypeError


class BaseHandler(object):
    def handle_get_options(self, data, s_id):
        pass

    def handle_evaluate(self, data, s_id):
        pass

    def handle_get_stats(self, data, s_id):
        pass

    def handle_save_output(self, data, s_id):
        pass


class BaseServerComm(object):
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

    def all_handle(self, handler=None):
        resp = self.receive()
        data = resp["data"]
        s_id = resp["id"]
        m_type = resp["type"]

        if m_type == MessageType.GET_OPTIONS:
            return handler.handle_get_options(data, s_id)
        if m_type == MessageType.EVALUATE:
            return handler.handle_evaluate(data, s_id)
        if m_type == MessageType.GET_STATS:
            return handler.handle_get_stats(data, s_id)
        if m_type == MessageType.SAVE_OUTPUT:
            return handler.handle_save_output(data, s_id)
        raise MessageTypeError

    def resp_options(self, data, s_id):
        self.send(data, s_id, MessageType.RESP_OPTIONS)

    def resp_score(self, data, s_id):
        self.send(data, s_id, MessageType.RESP_SCORE)

    def resp_stats(self, data, s_id):
        self.send(data, s_id, MessageType.RESP_STATS)

    def resp_save(self, data, s_id):
        self.send(data, s_id, MessageType.RESP_SAVE)

    def listen_forever(self, handler=None):
        while True:  # The famous Main Loop.
            # Wait for the request and handle it.
            self.all_handle(handler)


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


__all__ = ["LocalServerComm", "NetworkServerComm", "BaseHandler"]
