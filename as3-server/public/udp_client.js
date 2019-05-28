"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();


$(document).ready(function() {
	console.log('ready!');

	// setTimeout(socket.emit("servererrortest", "mode none\n") }, 10000);

	// mode buttons
	$('#modeNone').click(function(){
		console.log("none button clicked")
		socket.emit("modeCommand", "mode none\n");
	});
	
	$('#modeRock1').click(function(){
		console.log("rock1 clicked")
		socket.emit("modeCommand", "mode rock\n");
	});

	$('#modeRock2').click(function(){
		console.log("rock2 button clicked")
		socket.emit("modeCommand", "mode custom\n");
	});

	// volume & tempo buttons
	$('#volumeUp').click(function(){
		console.log("volume up button clicked")
		socket.emit("volumeCommand", "volumeup\n");
	});
	$('#volumeDown').click(function(){
		console.log("volume down button clicked")
		socket.emit("volumeCommand", "volumedw\n");
	});

	$('#tempoUp').click(function(){
		console.log("tempo up button clicked")
		socket.emit("tempoCommand", "tempoup\n");
	});

	$('#tempoDown').click(function(){
		console.log("tempo down button clicked")
		socket.emit("tempoCommand", "tempodw\n");
	});

	// sounds buttons
	$('#snare').click(function(){
		socket.emit("soundCommand", "play snare\n");
	});
	$('#base').click(function(){
		socket.emit("soundCommand", "play base\n");
	});
	$('#hihat').click(function(){
		socket.emit("soundCommand", "play hihat\n");
	});
	$('#kick').click(function(){
		socket.emit("soundCommand", "play kick\n");
	});
	$('#airdrum1').click(function(){
		socket.emit("soundCommand", "play airdrum1\n");
	});
	$('#airdrum2').click(function(){
		socket.emit("soundCommand", "play airdrum2\n");
	});
	$('#airdrum3').click(function(){
		socket.emit("soundCommand", "play airdrum3\n");
	});

	window.setInterval(function() {socket.emit("uptimeCommand", "get uptime\n");}, 1000);

	//listening
	socket.on('modeCommand', function(result) {
		console.log('modeCommand reply received : ' + result);
		$("#modeid").html(result);
	});

	socket.on('volumeCommand', function(result) {	
		var volume = parseInt(result);
		console.log('volumeCommand reply received ' + volume);
		$('#volumeid').val(volume);
	});

	socket.on('tempoCommand', function(result) {
		var tempo = parseInt(result);
		console.log('tempoCommand reply received ' + tempo);
		$('#tempoid').val(tempo);
	});

	socket.on('uptimeCommand', function(result) {
		console.log('uptimeCommand reply received : ' + result);
		$("#status").html(result);
	});

	// socket.on('volume_monitor_reply', function(result) {
	// 		console.log("result is " + result);
	// 		updateMonitor("#volumeid", result);
	// 	});
	// socket.on('tempo_monitor_reply', function(result) {
	// 		console.log("result is " + result);
	// 		updateMonitor("#tempoid", result);
	// 	});
	// socket.on('beat_monitor_reply', function(result) {
	// 	console.log("result is " + result);
	// 	updateMonitor("#mdoeid", result);
	// });
});

// function updateMonitor(key, value) {
// 	// var newDiv = $('<span></span>').text(value);
// 	$(key).val(value);
// }

// function sendCommand(command, message) {
// 	socket.emit(command, message);
// 	// console.log('command emitted');
// };
