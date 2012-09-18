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

var yourState = 'OFFLINE';
var theirState = 'OFFLINE';

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

sp.on("data", function (data) {
  var cmd = data.toString();
  console.log(">> ", util.inspect(cmd, true, null, true));
  switch (cmd) {
  case '~':
    chat = new Groupchat();
    chat.on('online', function () {
      writeToArduino('@');
      yourState = 'ONLINE';
    });
    chat.connect();
    break;
  case 'n':
    yourState = 'WAITING_TO_START';
    break;
  default:
    console.warn("Unknown message from Arduino: "+data);
  }
});

function writeToArduino (cmd) {
  console.log("<< ", util.inspect(cmd, true, null, true));
  sp.write(cmd);
}

function Groupchat () {
  events.EventEmitter.call(this);
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
  var sev = {
    type: type,
    payload: payload,
    timestamp: meta.timestamp || new Date(),
    origin: meta.origin || NICK
  };

  this.sendText(JSON.stringify(sev));
  return sev;
};