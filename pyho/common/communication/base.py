class MessageTypeError(ValueError):
    message = "Wrong response type from the transport."


class MessageType(object):
    GET_OPTIONS = 1
    RESP_OPTIONS = 101
    EVALUATE = 2
    RESP_SCORE = 102
    GET_STATS = 3
    RESP_STATS = 103
    SAVE_OUTPUT = 4
    RESP_SAVE = 104
