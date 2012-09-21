/*jshint node:true */

"use strict";

var util = require('util');
var events = require('events');
var serialport = require('serialport');
var xmpp = require('node-xmpp');

var sp;
var chat;

var HOST = "badger.encorelab.org";
var NICK = "toronto";
var PASSWORD = "toronto";

var MY_JID = NICK + "@" + HOST;
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

sp = new serialport.SerialPort('/dev/cu.usbmodemfd131');

function RPA () {
  events.EventEmitter.call(this);

  this.setYourState('OFFLINE');
  this.setTheirState('OFFLINE');
}
util.inherits(RPA, events.EventEmitter);

RPA.prototype.setYourState = function (newState) {
  console.log("MY STATE:", util.inspect(newState, true, null, true));
  this.yourState = newState;
};

RPA.prototype.setTheirState = function (newState) {
  console.log("THEIR STATE:", util.inspect(newState, true, null, true));
  this.theirState = newState;
};

RPA.prototype.readyToStartNewGame = function () {
  var rpa = this;
  if (this.theirState == 'ONLINE') {
    chat.sendEvent('ready');
    rpa.setYourState('READY');
  } else {
    chat.once('online', function () {
      chat.sendEvent('ready');
      rpa.setYourState('READY');
    });
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
  chat.sendEvent('choice', {weapon: c});
  this.yourChoice = c;
  this.setYourState('WAITING_FOR_OPPONENT');
};

RPA.prototype.check = function () {
  if (this.yourChoice === this.theirChoice) {
    this.setYourState('TIED');
    this.setTheirState('TIED');
    writeToArduino('=');
  } else if (this.yourChoice === "Rock" && this.theirChoice === "Scissors" ||
      this.yourChoice === "Paper" && this.theirChoice === "Rock" ||
      this.yourChoice === "Scissors" && this.theirChoice === "Paper") {
    this.setYourState('WON');
    this.setTheirState('LOST');
    writeToArduino('+');
  } else {
    this.setYourState('LOST');
    this.setTheirState('WON');
    writeToArduino('-');
  }
}

var rpa = new RPA();

rpa.on('ready', function (sev) {
  console.log("@ Other player is ready!");
  rpa.setTheirState('READY');
  writeToArduino('N');
});
rpa.on('choice', function (sev) {
  var weapon = sev.payload.weapon;
  console.log("@ Other player chose ", util.inspect(weapon, true, null, true));
  this.setTheirState("CHOSEN");
  this.theirChoice = weapon;
  if (this.yourState === 'WAITING_FOR_OPPONENT')
    this.check();
  else
    this.setYourState('WAITING_FOR_YOU');
});

var lastCmd;
sp.on("data", function (data) {
  var cmd = data.toString()[0];
  if (cmd === lastCmd) // cheap way to ignore duplicate input
    return;

  console.log(">> ", util.inspect(cmd, true, null, true));
  switch (cmd) {
  case '~':
    chat = new Groupchat();
    chat.on('event', function (sev) {
      rpa.emit(sev.eventType, sev);
    });
    chat.on('online', function () {
      writeToArduino('@');
      rpa.setYourState('ONLINE');
    });
    chat.on('joined', function () {
      rpa.setTheirState('ONLINE');
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
    console.warn("Unknown message from Arduino: "+data);
  }
  lastCmd = cmd;
});

function writeToArduino (cmd) {
  console.log("<< ", util.inspect(cmd, true, null, true));
  sp.write(cmd);
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
        var sev = JSON.parse(msg);
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
    origin: meta.origin || NICK
  };

  this.sendText(JSON.stringify(sev));
  return sev;
};