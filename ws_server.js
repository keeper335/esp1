/**
 * HTTP server for static content
 */
var path = require('path');
//var express = require('express');
//var app = express();

const http = require('http');
const server = http.createServer((req, res) => {
    if (req.method === 'POST') {
        let body = '';
        req.on('data', chunk => {
            body += chunk.toString(); // convert Buffer to string
        }); 
        req.on('end', () => {
            console.log(body);
            res.end("Ok");
        });
    }
    else {
        res.end();
    }
});
server.listen(3000);

const WSServer = new require('ws');
const wsServer = new WSServer.Server({port: 3025});

var clients = {};

function log(msg) {
    console.log('\x1B[36m' + msg + '\x1B[39m'); //don't spam to terminal
}

wsServer.on('connection', function(ws) {
    var id = Math.random().toString();
    clients[id] = ws;
    log('New connection: ' + id);
    ws.send("hello client\r\n");

    ws.on('message', function(message) {
        var i;
        log('Message from ' + id);
        log(message);
        if (typeof message === 'string' && !message.endsWith(0))
            message+=String.fromCharCode(0);

        try {
            //var msg_ = JSON.parse(message);
            for (i in clients) {
                if (i !== id) {
                    //          console.log('\t sent to %s', i);
                    clients[i].send(message);
                }
            }
        } catch (ee) {
            log(ee);
        }
    });

    ws.on('close', function() {
        console.log('Client %s was disconnected', id);
        delete clients[id];
    });
});

process.on('uncaughtException', function(err) {
    //stop closing my window, fkn zlib!
    log('Caught exception: ' + err);
});

/* Client */
/* socket = new WebSocket('ws://127.0.0.1:3025', 'blade-protocol-v1');
     socket.onmessage = (data) => {console.log(data);}
*/