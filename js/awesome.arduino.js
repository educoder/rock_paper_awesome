/*jshint node:true */

"use strict";

var fs = require('fs');
var serialport = require('serialport');
var color = require('cli-color');
var util = require('util');
var events = require('eventemitter2');

function Arduino (serialportDev) {
  events.EventEmitter2.call(this);
  this.serialportDev = serialportDev || undefined;
}
util.inherits(Arduino, events.EventEmitter2);

Arduino.prototype.detectSerialport = function () {
  if (this.serialportDev) {
    console.log("Using hardcoded SERIALPORT for Arduino dev: ", this.serialportDev);
    return;
  }

  var devs = fs.readdirSync('/dev/');
  var possibleArduinoDevs = [];
  for (var i in devs) {
    if (devs[i].match(/^tty.*?(USB|ACM).*?/i)) {
      possibleArduinoDevs.push(devs[i]);
    }
  }

  if (possibleArduinoDevs.length > 0) {
    console.log("Found possible Arduinos:");
    for (var j in possibleArduinoDevs) {
      console.log("  ", possibleArduinoDevs[j]);
    }
    console.log("");
    var arduinoDev = possibleArduinoDevs[0];
    console.log("Using: ", arduinoDev);
    console.log("");
    this.serialportDev = "/dev/"+arduinoDev;
  } else {
    console.error("No Arduino dev found. Please make sure it is plugged in or hardcode a SERIALPORT value in "+__filename+".");
    this.serialportDev = undefined;
  }

};

Arduino.prototype.openSerialport = function () {
  this.serialport = new serialport.SerialPort(this.serialportDev, {
    parser: serialport.parsers.readline("\r\n")
  });  
  this.bindInputProcessor();
};

Arduino.prototype.bindInputProcessor = function () {
  var arduino = this;

  this.serialport.on("data", function (data) {
    var input = data.toString();
    arduino.processInput(input);
  });
};

Arduino.prototype.writeEvent = function(event, arg) {
  if (arg)
    this.writeToSerialport("> "+event+" "+arg);
  else
    this.writeToSerialport("> "+event);
};

Arduino.prototype.writeState = function(state) {
  this.writeToSerialport("= "+state);
};

Arduino.prototype.writeToSerialport = function(data) {
  console.log(color.blackBright(">> [TO ARDUINO]"), util.inspect(data, true, null, true));
  this.serialport.write(data+"\n");
  this.serialport.flush(); // might not be necessary?
};

Arduino.prototype.processInput = function (input) {
  console.log(color.blackBright("<< [FROM ARDUINO]"), util.inspect(input, true, null, true));
  
  var c = input.split(" ");
  var ctrl = c[0];
  var ev = c[1];
  var arg = c[2];

  if (input === '~') {
    this.emit('online');
    //rpa.prepare();
  } else if (ctrl === '<') {
    // NOTE: arg can be undefined
    this.emit(ev, arg);
    //rpa.fsm.handle(cmd, arg);
  } else {
    console.warn(color.red("!!! Unknown message from Arduino: "), input.toString());
  }
  // switch (cmd) {
  // case '~':
  //   rpa.enterDojo();
  //   break;
  // case 'n': 
  //   rpa.bow();
  //   break;
  // case 'r':
  // case 'p':
  // case 's':
  //   rpa.setYourWeapon(cmd);
  //   break;
  // default:
  //   var match;
  //   if (match = cmd.match(/\[(\d)/)) {
  //     arduinoStateChange(parseInt(match[1]));
  //   } else if (match = cmd.match(/!(.+)/)) {
  //     arduinoInvalidTransition(match[1]);
  //   } else {
  //     console.warn(color.red("!!! Unknown message from Arduino: "), cmd.toString());
  //   }
  // }
};


exports.Arduino = Arduino;