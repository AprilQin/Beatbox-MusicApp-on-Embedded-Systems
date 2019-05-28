var http = require('http');
var mime = require('mime');
var path = require('path');
var PORT = 3042;
var server = http.createServer(function (request, response) {
	// body...
	var filePath = false;
	if (request.url == '/') {
		filePath = 'public/index.html'; 
	} else { 
		filePath = 'public' + request.url;
	}
	var absPath = './' + filePath;
	serveStatic(response, absPath);
});


server.listen(PORT, function() {
	console.log("Server listening on port " + PORT);
});

var fs = require('fs');

function serveStatic(response, absPath){
	fs.exists(absPath, function(exists){
		if (exists) {
			fs.readFile(absPath, function(err, data){
				if (err){
					send404(response);
				} else {
					sendFile(response, absPath, data);
				}
			});
		}
		else {
			send404(response);
		}
	});
}

function send404(response) {
	response.writeHead(404, {'Content-Type': 'text/plain'});
	response.write('Error 404: resource not found.');
	response.end();
}

function sendFile(response, filePath, fileContents) {
	response.writeHead(
			200,
			{"content-type": mime.lookup(path.basename(filePath))}
		);
	response.end(fileContents);
}

var udpServer = require('./lib/udp_server');
udpServer.listen(server);

