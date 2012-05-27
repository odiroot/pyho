class MessageTypeError(ValueError):
    message = "Wrong response type from the transport."


class MessageType(object):
    GET_OPTIONS = 1
    RESP_OPTIONS = 2
    EVALUATE = 3
    RESP_SCORE = 4
    GET_STATS = 5
    RESP_STATS = 6
    SAVE_OUTPUT = 7
    RESP_SAVE = 8
