"use strict";
/*
 * "io.sockets.on" listens(receive messages) to browser side socket io command messages
 	"handleCommand" handles those command messages
 	"var client" creates udp connection, "client.send" send packets and "client.on" listens for packets
	"socket.emit" respond(send messages) to browser side socket io
 */

var socketio = require('socket.io');
var io;

var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	
	io.sockets.on('connection', function(socket) {
		console.log('socket io listening');
		handleCommand(socket, "modeCommand");
		handleCommand(socket, "volumeCommand");
		handleCommand(socket, "tempoCommand");
		handleCommand(socket, "soundCommand");
		handleCommand(socket, "uptimeCommand");
		
	});
};

function handleCommand(socket, command) {
	socket.on(command, function(data) {
		console.log(command + ':'+ data);
		
		// Info for connecting to the local process via UDP
		var PORT = 12345;
		var HOST = '192.168.7.2';
		var buffer = new Buffer(data);

		var client = dgram.createSocket('udp4');
		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		    if (err) 
		    	throw err;
		    // console.log('udp packets to ' + HOST +':'+ PORT + buffer);
		});
		
		client.on('listening', function () {
		    var address = client.address();
		    // console.log('listening on ' + address.address + ":" + address.port); //my socket io session address and port
		});

		client.on('message', function (message) {
		    var reply = message.toString('utf8');
		    // console.log('reply for ' + command + 'received from beatboxplayer');
		    // console.log('reply is ' + reply)
		    socket.emit(command, reply);
		    client.close();
		});
		
		// client.on("UDP Client: error", function(err) {
		//     console.log("error: ",err);
		// });

	});
}
