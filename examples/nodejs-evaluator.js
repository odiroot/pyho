#!/usr/bin/env node
var zmq = require('zmq');
var sys = require('sys');


var MessageType = {
    GET_OPTIONS: 1,
    RESP_OPTIONS: 2,
    EVALUATE: 3,
    RESP_SCORE: 4,
    GET_STATS: 5,
    RESP_STATS: 6,
    SAVE_OUTPUT: 7,
    RESP_SAVE: 8
};


var parseAddresses = function() {
    var ret = {};
    process.argv.forEach(function (val, index, array) {
        if(val === "-local-pull-address") {
            ret["pull"] = array[index + 1];
        }
        if(val === "-local-publish-address") {
            ret["pub"] = array[index + 1];
        }
    });
    return ret
};

var createSockets = function(addresses) {
    var listener = zmq.socket("pull");
    listener.connect(addresses["pull"]);
    var publisher = zmq.socket("pub")
    publisher.connect(addresses["pub"]);

    return {
        "listener": listener,
        "publisher": publisher
    }
};


var handle_options = function(data) {
    return {
        "num_params": 5,
        "min_constr": [-1, -2, -3, -4, -5],
        "max_constr": [5, 4, 3, 2, 1]
    };
};

var handle_evaluate = function(data) {
    var params = data.params;
    var sum = 0;
    params.forEach(function(val, index, array) {
        sum += val;
    });
    return {
        "score": Math.abs(sum / params.length)
    };
}

var handle_message = function(msg, publisher) {
    console.log("Received message: %s", msg.toString());

    var parsed = JSON.parse(msg);
    var type = parseInt(parsed["type"], 10);
    var data = parsed["data"];

    var res = null;
    var mType = null;

    switch(type) {
        case MessageType.GET_OPTIONS:
            res = handle_options(data);
            mType = MessageType.RESP_OPTIONS;
        break;
        case MessageType.EVALUATE:
            res = handle_evaluate(data);
            mType = MessageType.RESP_SCORE;
            console.log("SCORE: %s", res.score);
        break;
        default:
            console.error("Unknown message type: %s", type);
    }

    var body = JSON.stringify({
        "data": res || {},
        "id": parsed["id"],
        "type": mType || 0
    });
    publisher.send(body);
};


var run = function() {
    sys.print("Starting dummy evaluator.\n");
    var socks = createSockets(parseAddresses());

    socks.listener.on("message", function(msg) {
        handle_message(msg, socks.publisher);
    });
};

run();


