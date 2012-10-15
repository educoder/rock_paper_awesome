/*jshint node:true */

"use strict";

var SERIALPORT = '/dev/tty.usbmodemfa141';
//var SERIALPORT = '/dev/tty.usbserial-A6006klc';

// FIXME: this is in bad need of a rewrite

var util = require('util');
var events = require('events');
var serialport = require('serialport');
var xmpp = require('node-xmpp');
var color = require('cli-color');

var os = require('os');
var exec = require('child_process').exec;


var sp;
var chat;

var HOST = "badger.encorelab.org";
var USER = "toronto";
var NICK = process.argv[2] || USER;
var PASSWORD = "toronto";

var MY_JID = USER + "@" + HOST;
var ROOM_JID = "rock-paper-awesome@conference." + HOST;
var MY_JID_IN_ROOM = ROOM_JID + "/" + NICK;

// can't get this auto-detection code to work on Mac... port.comName seems to be wrong
//
// serialport.list(function (err, ports) {
//   ports.forEach(function(port) {
//     if (port.manufacturer.indexOf("Arduino") >= 0) {
//       sp = new serialport.SerialPort(port.comName.toString());
//     }
//   });
// });

sp = new serialport.SerialPort(SERIALPORT, {
  parser: serialport.parsers.readline("\r\n")
});

function RPA () {
  events.EventEmitter.call(this);

  this.yourStateChangedTo('OFFLINE');
  this.theirStateChangedTo('OFFLINE');
}
util.inherits(RPA, events.EventEmitter);

RPA.prototype.yourStateChangedTo = function (newState) {
  console.log("*----------------------[ MY Node's STATE:", util.inspect(newState, true, null, true));
  this.yourState = newState;
};

RPA.prototype.theirStateChangedTo = function (newState) {
  console.log("*--------------------------------------------------------[ THEIR Node's STATE:", util.inspect(newState, true, null, true));
  this.theirState = newState;
};

RPA.prototype.readyToStartNewGame = function () {
  var rpa = this;
  this.yourChoice = null;
  if (this.theirState === 'OFFLINE') {
    console.log("Waiting for other player to come online...");
    chat.once('online', function () {
      console.log("Other player is ONLINE, telling them that we're ready!");
      chat.sendEvent('ready');
    });
  } else if (this.theirState === 'READY') {
    console.log("Other player is READY, telling them that we're also ready!");
    chat.sendEvent('ready');
  } else {
    console.log("Other player is ONLINE, telling them that we're ready!");
    chat.sendEvent('ready');
  }
};

RPA.prototype.youChoose = function (choice) {
  var c;
  switch (choice) {
  case 'r':
    c = 'Rock';
    break;
  case 'p':
    c = 'Paper';
    break;
  case 's':
    c = 'Scissors';
    break;
  }
  console.log("~~~ You chose ", util.inspect(c, true, null, true));
  this.yourChoice = c;
  if (this.yourState === 'WAITING_FOR_YOUR_CHOICE') {
    var self = this;
    this.once('arduino_state_WAITING_FOR_RESULT', function () {
      self.check();
    });
  }
  chat.sendEvent('choice', {weapon: c});
};

RPA.prototype.check = function () {
  if (this.yourChoice === this.theirChoice) {
    writeToArduino('=');
  } else if (this.yourChoice === "Rock" && this.theirChoice === "Scissors" ||
      this.yourChoice === "Paper" && this.theirChoice === "Rock" ||
      this.yourChoice === "Scissors" && this.theirChoice === "Paper") {
    writeToArduino('+');
  } else {
    writeToArduino('-');
  }
}

RPA.prototype.processInputFromArduino = function (cmd) {
  console.log(color.blackBright("<< [FROM ARDUINO]"), util.inspect(cmd, true, null, true));
  switch (cmd) {
  case '~':
    chat = new Groupchat();
    chat.on('event', function (sev) {
      rpa.emit(sev.eventType, sev);
    });
    chat.on('online', function () {
      writeToArduino('@');
    });
    chat.on('joined', function () {
      rpa.theirStateChangedTo('ONLINE');
    });
    chat.connect();
    break;
  case 'n': 
    rpa.readyToStartNewGame();
    break;
  case 'r':
  case 'p':
  case 's':
    rpa.youChoose(cmd);
    break;
  default:
    var match;
    if (match = cmd.match(/\[(\d)/)) {
      arduinoStateChange(parseInt(match[1]));
    } else if (match = cmd.match(/!(.+)/)) {
      arduinoInvalidTransition(match[1]);
    } else {
      console.warn(color.red("!!! Unknown message from Arduino: "), cmd.toString());
    }
  }
}

var rpa = new RPA();

rpa.on('ready', function (sev) { // remote_ready
  console.log("~~~ Other player is ready!");
  rpa.theirStateChangedTo('READY');
  this.theirChoice = null;
  writeToArduino('N');
});
rpa.on('choice', function (sev) {
  var weapon = sev.payload.weapon;
  console.log("~~~ Other player chose ", util.inspect(weapon, true, null, true));
  this.theirStateChangedTo("CHOSEN");
  this.theirChoice = weapon;
  if (this.yourState === 'WAITING_FOR_THEIR_CHOICE') {
    var self = this;
    this.once('arduino_state_WAITING_FOR_RESULT', function () {
      self.check();
    });
  }
  writeToArduino(weapon[0]);
});

sp.on("data", function (data) {
  var input = data.toString();
  rpa.processInputFromArduino(input);
});

function arduinoStateChange (stateNum) {

  // IMPORTANT: must keep this in sync with what's in RPA.h!
  var arduinoStates = {
    0: "OFFLINE",
    1: "CONNECTING",
    2: "ONLINE",
    3: "WAITING_FOR_THEIR_READY",
    4: "WAITING_FOR_YOUR_READY",
    5: "READY_TO_PLAY",
    6: "WAITING_FOR_THEIR_CHOICE",
    7: "WAITING_FOR_YOUR_CHOICE",
    8: "WAITING_FOR_RESULT"
  };

  console.log("*-[ MY Arduino's STATE: ", util.inspect(arduinoStates[stateNum], true, null, true));
  rpa.yourStateChangedTo(arduinoStates[stateNum]);
  rpa.emit("arduino_state_"+arduinoStates[stateNum]);
}

function arduinoInvalidTransition (eventName) {
  console.error(color.redBright("!!! Arduino says that an invalid event was triggered:"), util.inspect(eventName, true, null, true));
}

var writeToSerialport;
if (os.arch() == 'arm') {
  // FIXME: hacky way to write to serialport on Raspberry Pi....
  //        sp.write() doesn't seem to work for some reason :(
  writeToSerialport = function (data) {
    exec("echo '"+data+"' > "+SERIALPORT);
  }
} else {
  writeToSerialport = function (data) {
    sp.write(data);
  }
}


function writeToArduino (cmd) {
  console.log(color.blackBright(">> [TO ARDUINO]"), util.inspect(cmd, true, null, true));
  writeToSerialport(cmd);
}


function Groupchat () {
  events.EventEmitter.call(this);
  this.roster = [];
}
util.inherits(Groupchat, events.EventEmitter);

Groupchat.prototype.connect = function () {
  var chat = this;
  chat.client = new xmpp.Client({jid: MY_JID, password: PASSWORD});
  chat.client.on('online', function () {
    console.log("XMPP: Connected to " + MY_JID);
    chat.emit('connected');

    chat.client.on('stanza', function (stanza) {
      if (stanza.is('presence') && stanza.attrs.from == MY_JID_IN_ROOM) {
        console.log("XMPP: Joined room "+ROOM_JID);
        chat.emit('online');
      } else if (stanza.is('presence') && stanza.attrs.from != MY_JID_IN_ROOM) {
        console.log("XMPP: "+util.inspect(stanza.attrs.from, true, null, true)+" joined "+ROOM_JID);
        chat.roster.push(stanza.attrs.from);
        chat.emit('joined', stanza.attrs.from);
        // TODO: implement 'left'
      } else if (stanza.is('message') && stanza.attrs.type == 'groupchat' &&
          stanza.attrs.from != MY_JID_IN_ROOM) {
        var msg = stanza.getChildText('body');
        console.log("XMPP: Got message "+msg, stanza.attrs.from);
        try {
          var sev = JSON.parse(msg);
        } catch (err) {
          console.warn("Could not parse event mesage! ("+msg+")");
          return;
        }
        chat.emit('event', sev);
      }
    });

    chat.client.send(new xmpp.Element('presence', { to: MY_JID_IN_ROOM }).
      c('x', { xmlns: 'http://jabber.org/protocol/muc' }));
    chat.emit('connecting');
  });
};

Groupchat.prototype.sendText = function (text) {
  chat.client.send(new xmpp.Element('message', { to: ROOM_JID, type: 'groupchat' }).
    c('body').t(text));
};

Groupchat.prototype.sendEvent = function (type, payload, meta) {
  if (payload === undefined)
    payload = {};
  if (meta === undefined)
    meta = {};

  var sev = {
    eventType: type,
    payload: payload,
    timestamp: meta.timestamp || new Date(),
    origin: meta.origin || USER
  };

  this.sendText(JSON.stringify(sev));
  return sev;
};