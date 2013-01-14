/*jshint node:true */

"use strict";

var SERIALPORT = undefined;
//var SERIALPORT = '/dev/tty.usbmodemfd131';
//var SERIALPORT = '/dev/tty.usbserial-A6006klc';

// FIXME: this is in bad need of a rewrite

var util = require('util');
var events = require('eventemitter2');
var color = require('cli-color');

var os = require('os');
var fs = require('fs');
var exec = require('child_process').exec;

var sp;
var chat;

var HOST = process.argv[4] || "badger.encorelab.org";
var USER = process.argv[2];
var NICK = USER;
var PASSWORD = process.argv[3];

if (!PASSWORD) {
  console.error("\nUsage: node awesome.js USERNAME PASSWORD [HOSTNAME]\n");
  process.exit(1);
}


function RPA () {
  events.EventEmitter2.call(this);

  this.arduinoSaidHello = false;

  this.winCount = 0;
  this.loseCount = 0;

  this.opponent = undefined;

  this.yourWeapon = undefined;
  this.theirWeapon = undefined;
}
util.inherits(RPA, events.EventEmitter2);

RPA.Arduino = require('./js/awesome.arduino').Arduino;
RPA.Groupchat = require('./js/awesome.xmpp').Groupchat;

RPA.prototype.connectToArduino = function () {
  this.arduino = new RPA.Arduino();

  var rpa = this;

  function log(msg, data) {
    if (data)
      console.log(color.yellowBright("[Arduino] "+msg), util.inspect(data, true, null, true));
    else
      console.log(color.yellowBright("[Arduino] "+msg));
  }

  this.arduino
    .on('online', function () {
      log("Arduino is online");
    })
    .on('present', function () {
      log("You are now present");

      if (!rpa.chat) {
        rpa.connectToGroupchat();
        rpa.chat.once('connected', function () {
          rpa.chat.enter();
        });
      } else {
        rpa.chat.enter();
      }

      rpa.arduino.writeEvent('you_are_present');
    })
    .on('absent', function () {
      log("You are now absent");

      rpa.chat.leave();
      rpa.arduino.writeEvent('you_are_absent');
    })
    .on('you_choose', function (weapon) {
      rpa.yourWeapon = weapon;
      rpa.chat.sendEvent('choice', weapon);
      rpa.arduino.writeEvent('you_choose_weapon', weapon[0]);
      rpa.checkOutcome();
    });


  this.arduino.detectSerialport();
  this.arduino.openSerialport();
};

RPA.prototype.connectToGroupchat = function () {
  this.chat = new RPA.Groupchat(USER, PASSWORD, HOST, NICK);

  var rpa = this;

  function log(msg, data) {
    if (data)
      console.log(color.magentaBright("[XMPP] "+msg), util.inspect(data, true, null, true));
    else
      console.log(color.magentaBright("[XMPP] "+msg));
  }

  this.chat.log = log;

  // this.chat.onAny(function (sev) {
  //   //rpa.arduino.emit(this.event, data);
  //   if (sev && sev.payload)
  //     rpa.arduino.writeEvent(this.event, sev.payload);
  //   else
  //     rpa.arduino.writeEvent(this.event);
  // });

  this.chat
    .on('opponent_joined_groupchat', function (jid) {
      rpa.opponent = jid;
      rpa.arduino.writeEvent('opponent_is_present', jid.resource);
    })
    .on('sail_event', function (sev) {
      var eventType = sev.eventType;
      var payload = sev.payload;

      if (eventType == 'choice') {
        log("Opponent chose ", payload);
        rpa.theirWeapon = payload;

        rpa.arduino.writeEvent('opponent_chooses_weapon', payload[0]);
        rpa.checkOutcome();
      }
    });

  this.chat.connect();
};

RPA.prototype.checkOutcome = function () {
  if (this.yourWeapon && this.theirWeapon) {
    if (this.yourWeapon === this.theirWeapon) {
      this.arduino.writeEvent('tie');
    } else if (this.yourWeapon === "Rock" && this.theirWeapon === "Scissors" ||
        this.yourWeapon === "Paper" && this.theirWeapon === "Rock" ||
        this.yourWeapon === "Scissors" && this.theirWeapon === "Paper") {
      this.arduino.writeEvent('you_win');
      this.winCount++;
    } else {
      this.arduino.writeEvent('you_lose');
      this.loseCount++;
    }

    this.yourWeapon = undefined;
    this.theirWeapon = undefined;

    // TODO: should we really do this?
    rpa.arduino.writeEvent('you_are_present');
    rpa.arduino.writeEvent('opponent_is_present');
  }
};

var rpa = new RPA();
rpa.connectToArduino();
