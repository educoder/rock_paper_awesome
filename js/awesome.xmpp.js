/*jshint node:true */

"use strict";

var events = require('eventemitter2');
var util = require('util');
var xmpp = require('node-xmpp');
var color = require('cli-color');

function Groupchat(user, password, host, nick) {
  events.EventEmitter2.call(this);

  this.user = user;
  this.password = password;
  this.host = host;
  this.nick = nick || user;

  this.myJid = this.user + "@" + this.host;
  this.roomJid = "rpa@conference." + this.host;
  this.myJidInRoom = this.roomJid + "/" + this.nick;

  this.log = function (msg, data) {
    if (data)
      console.log("XMPP: "+msg, data);
    else
      console.log("XMPP: "+msg);
  };

  this.roster = [];
}
util.inherits(Groupchat, events.EventEmitter2);

Groupchat.prototype.connect = function () {
  var chat = this;
  this.log("Connecting to XMPP Groupchat as ",this.myJid);
  
  this.emit('connecting');

  chat.client = new xmpp.Client({jid: chat.myJid, password: chat.password});

  chat.client.on('error', function (error) {
    console.error("XMPP ERROR: ",color.red(error));
  });

  chat.client.on('online', function () {
    chat.log("Connected to " + chat.myJid);
    chat.emit('connected');

    chat.client.on('stanza', function (stanza) {
      if (stanza.is('presence') && stanza.attrs.from == chat.myJidInRoom) {

        if (stanza.attr('type') == "unavailable") {
          chat.log("YOU left groupchat "+chat.roomJid);
          chat.emit('you_left_groupchat');
        } else {
          chat.log("YOU joined groupchat "+chat.roomJid);
          chat.emit('you_joined_groupchat');
        }

      } else if (stanza.is('presence') && stanza.attrs.from != chat.myJidInRoom) {
        
        if (stanza.attr('type') == "unavailable") {
          chat.log("Another player left "+chat.roomJid, stanza.attrs.from);
          chat.roster.splice(chat.roster.indexOf(stanza.attrs.from), 1);
          chat.emit('opponent_left_groupchat', new xmpp.JID(stanza.attrs.from));
        } else {
          chat.log("Another player joined "+chat.roomJid, stanza.attrs.from);
          chat.roster.push(stanza.attrs.from);
          chat.emit('opponent_joined_groupchat', new xmpp.JID(stanza.attrs.from));
        }

      } else if (stanza.is('message') && stanza.attrs.type == 'groupchat' &&
          stanza.attrs.from != chat.myJidInRoom) {
        var msg = stanza.getChildText('body');
        chat.log("Got message from "+stanza.attrs.from, msg);
        var sev;
        try {
          sev = JSON.parse(msg);
        } catch (err) {
          console.warn("Could not parse event mesage! ("+msg+")");
          return;
        }
        chat.emit('sail_event', sev);
      }
    });
  });
};

Groupchat.prototype.enter = function () {
  var chat = this;
  chat.client.send(new xmpp.Element('presence', { to: chat.myJidInRoom }).
      c('x', { xmlns: 'http://jabber.org/protocol/muc' }));
}

Groupchat.prototype.leave = function () {
  var chat = this;
  chat.client.send(new xmpp.Element('presence', { to: chat.myJidInRoom, type: "unavailable" }).
      c('x', { xmlns: 'http://jabber.org/protocol/muc' }));
}

Groupchat.prototype.sendText = function (text) {
  this.client.send(new xmpp.Element('message', { to: this.roomJid, type: 'groupchat' }).
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
    origin: meta.origin || this.user
  };

  this.sendText(JSON.stringify(sev));
  return sev;
};

exports.Groupchat = Groupchat;