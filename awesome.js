/*jshint node:true */

"use strict";

var SERIALPORT = undefined;
//var SERIALPORT = '/dev/tty.usbmodemfd131';
//var SERIALPORT = '/dev/tty.usbserial-A6006klc';

// FIXME: this is in bad need of a rewrite

var util = require('util');
var events = require('events');
var serialport = require('serialport');
var xmpp = require('node-xmpp');
var color = require('cli-color');
var machina = require('machina');

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

function detectArduino() {
  if (SERIALPORT) {
    console.log("Using hardcoded SERIALPORT for Arduino dev: ", SERIALPORT);
    return SERIALPORT;
  }

  var devs = fs.readdirSync('/dev/');
  var possibleArduinoDevs = [];
  for (var i in devs) {
    if (devs[i].match(/^tty.*?(USB|ACM).*?/i)) {
      possibleArduinoDevs.push(devs[i]);
    }
  }

  if (possibleArduinoDevs.length > 0) {
    console.log("Found possible Arduinos:")
    for (var i in possibleArduinoDevs) {
      console.log("  ", possibleArduinoDevs[i]);
    }
    console.log("");
    var arduinoDev = possibleArduinoDevs[0];
    console.log("Using: ", arduinoDev);
    console.log("");
    return "/dev/"+arduinoDev;
  } else {
    console.error("No Arduino dev found. Please make sure it is plugged in or hardcode a SERIALPORT value in "+__filename+".");
    return undefined;
  }
}

sp = new serialport.SerialPort(detectArduino(), {
  parser: serialport.parsers.readline("\r\n")
});

function RPA () {
  events.EventEmitter.call(this);

  this.arduinoSaidHello = false;

  this.winCount = 0;
  this.loseCount = 0;

  var rpa = this;
  setTimeout(function () {
    if (!rpa.arduinoSaidHello) {
      console.log("Waited 10 seconds but haven't heard from Arduino... will try to go online anyway.");
      rpa.enterDojo();
    }
  }, 10000);
}
util.inherits(RPA, events.EventEmitter);


RPA.Fsm = machina.Fsm.extend({
  initialize: function () {

  },
  eventListeners: {
    disconnected: [function () { 
      this.transition('OFFLINE') 
    }]
  },
  initialState: 'OFFLINE',
  states: {
    'OFFLINE': {
      connecting: function () {
        this.transition('CONNECTING');
      },
      connected: function () {
        this.transition('ONLINE');
      }
    },
    'CONNECTING': {
      connected: function () {
        this.transition('ONLINE');
      }
    },
    'ONLINE': {
      ready: function () {
        rpa.bow();
        this.transition('WAITING_FOR_THEIR_READY');
      },
      remote_ready: function () {
        this.transition('WAITING_FOR_YOUR_READY');
      }
    },
    'WAITING_FOR_YOUR_READY': {
      ready: function () {
        rpa.bow();
        this.transition('READY_TO_PLAY');
      }
    },
    'WAITING_FOR_THEIR_READY': {
      remote_ready: function () {
        this.transition('READY_TO_PLAY');
      }
    },
    'READY_TO_PLAY': {
      you_choose: function (weapon) {
        rpa.setYourWeapon(weapon);
        this.transition('WAITING_FOR_THEIR_CHOICE');
      },
      they_choose: function (weapon) {
        this.transition('WAITING_FOR_YOUR_CHOICE');
      }
    },
    'WAITING_FOR_THEIR_CHOICE': {
      they_choose: function(weapon) {
        this.transition('WAITING_FOR_RESULT');
      }
    },
    'WAITING_FOR_YOUR_CHOICE': {
      you_choose: function (weapon) {
        rpa.setYourWeapon(weapon)
        this.transition('WAITING_FOR_RESULT');
      }
    },
    'WAITING_FOR_RESULT': {
      _onEnter: function () {
        rpa.checkOutcome();
      },
      you_win: function () {
        this.winCount++;
        this.transition('ONLINE');
      },
      you_lose: function () {
        this.loseCount++;
        this.transition('ONLINE');
      },
      tie: function () {
        this.transition('ONLINE');
      }
    }
  },

  sendEventToArduino: function(event, arg) {
    if (arg)
      writeToArduino("> "+event+" "+arg);
    else
      writeToArduino("> "+event);
  },

  sendStateToArduino: function(state) {
    writeToArduino("= "+state);
  }
});


RPA.prototype.yourStateChangedTo = function (newState) {
  console.log("*----------------------[ MY Node's STATE:", util.inspect(newState, true, null, true));
  this.yourState = newState;
};

RPA.prototype.theirStateChangedTo = function (newState) {
  console.log("*--------------------------------------------------------[ THEIR Node's STATE:", util.inspect(newState, true, null, true));
  this.theirState = newState;
};

RPA.prototype.bow = function () {
  var rpa = this;
  this.yourWeapon = null;
  if (this.theirState === 'OFFLINE') {
    console.log("Waiting for other player to come online...");
    chat.once('i_entered_room', function () {
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

RPA.prototype.setYourWeapon = function (weapon) {
  console.log("~~~ You chose ", util.inspect(weapon, true, null, true));
  this.yourWeapon = weapon;
  chat.sendEvent('choice', {weapon: weapon});
};

RPA.prototype.checkOutcome = function () {
  if (this.yourWeapon === this.theirWeapon) {
    this.fsm.handle('tie');
  } else if (this.yourWeapon === "Rock" && this.theirWeapon === "Scissors" ||
      this.yourWeapon === "Paper" && this.theirWeapon === "Rock" ||
      this.yourWeapon === "Scissors" && this.theirWeapon === "Paper") {
    this.fsm.handle('you_win');
  } else {
    this.fsm.handle('you_lose');
  }
}

RPA.prototype.enterDojo = function () {

  if (!this.fsm) {
    console.log("Initializing FSM...");
    var fsm = new RPA.Fsm();
    fsm.on('*', function (type, info) { 
      if (type === 'transition') {
        console.log("TRANSITION==[", color.xterm(202)(info.fromState), '----'+fsm.currentActionArgs[0]+'---->', color.greenBright(info.toState));
        fsm.sendStateToArduino(info.toState);
      } else if (type === 'handling') {
        console.log("EVENT=======[", color.xterm(51)(info.inputType), info.args);
        fsm.sendEventToArduino(info.inputType, info.args[0]);
      } else if (type === 'handled') {
        // nothing right now...
      } else if (type === 'nohandler') {
        console.error(color.red("!!!"),color.black.bgRed(info.inputType),color.red("is not a valid event at this time!"));
      } else {
        console.log(type, info);
      }
    });
    var rpa = this;

    fsm.on('ready', function () {
      rpa.bow();
    });

    this.fsm = fsm;
    chat = new Groupchat();
    chat.on('event', function (sev) {
      rpa.emit(sev.eventType, sev);
    });
    chat.on('i_entered_room', function () {
      fsm.handle('connected');
    });
    chat.on('they_entered_room', function () {
      console.log("Another player entered the room.");
      // fsm.handle('connected');
    });
  }
  
  chat.connect();
  this.arduinoSaidHello = true;
}

RPA.prototype.processInputFromArduino = function (input) {
  console.log(color.blackBright("<< [FROM ARDUINO]"), util.inspect(input, true, null, true));
  
  var c = input.split(" ");
  var ctrl = c[0];
  var cmd = c[1];
  var arg = c[2];

  if (input === '~') {
    this.enterDojo();
  } else if (ctrl === '<') {
    this.fsm.handle(cmd, arg);
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
}

var rpa = new RPA();

rpa.on('ready', function (sev) { // remote_ready
  console.log("~~~ Other player is ready!");
  rpa.theirStateChangedTo('READY');
  this.theirWeapon = null;
  rpa.fsm.handle('remote_ready');
});
rpa.on('choice', function (sev) {
  var weapon = sev.payload.weapon;
  console.log("~~~ Other player chose ", util.inspect(weapon, true, null, true));
  this.theirStateChangedTo("CHOSEN");
  this.theirWeapon = weapon;
  rpa.fsm.handle('they_choose', weapon);
});

sp.on("data", function (data) {
  var input = data.toString();
  rpa.processInputFromArduino(input);
});

function writeToArduino (data) {
  console.log(color.blackBright(">> [TO ARDUINO]"), util.inspect(data, true, null, true));
  sp.write(data+"\n");
  sp.flush(); // might not be necessary?
}

function Groupchat () {
  events.EventEmitter.call(this);
  this.roster = [];
}
util.inherits(Groupchat, events.EventEmitter);

Groupchat.prototype.connect = function () {
  var chat = this;
  console.log("Connecting to XMPP Groupchat as ",MY_JID,"...")
  rpa.fsm.handle('connecting');

  chat.client = new xmpp.Client({jid: MY_JID, password: PASSWORD});

  chat.client.on('error', function (error) {
    console.error("XMPP ERROR: ",color.red(error));
  });

  chat.client.on('online', function () {
    console.log("XMPP: Connected to " + MY_JID);
    chat.emit('connected');

    chat.client.on('stanza', function (stanza) {
      if (stanza.is('presence') && stanza.attrs.from == MY_JID_IN_ROOM) {
        console.log("XMPP: Joined room "+ROOM_JID);
        chat.emit('i_entered_room');

      } else if (stanza.is('presence') && stanza.attrs.from != MY_JID_IN_ROOM) {
        console.log("XMPP: "+util.inspect(stanza.attrs.from, true, null, true)+" joined "+ROOM_JID);
        chat.roster.push(stanza.attrs.from);
        chat.emit('they_entered_room', stanza.attrs.from);
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