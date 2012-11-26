#include "RPA.h"

#define SERIAL_BAUDRATE 9600

#define LED 11

// instantiate a RockPaperAwesome state machine
RPA rpa = RPA();


/*******************************************************************/
/** EVENT HANDLERS *************************************************/
/*******************************************************************/

// when the other player has signaled that they're ready to play a game
void on_remote_ready() {
  delay(1000);
  rpa.bow();
}

// when the other player has chosen a weapon
void on_they_choose(String weapon) {
  delay(1000);
  int rand = random(1,3);
  switch (rand) {
    case 1:
      rpa.choose_ROCK();
      break;
     case 2:
       rpa.choose_PAPER();
       break;
     case 3:
       rpa.choose_SCISSORS();
       break;
  }
}

// when you've won
void on_you_win() {
  blink(LED, 10, 100);
}

// when you've lost
void on_you_lose() {
  blink(LED, 3, 1000);
}

// when you've tied
void on_tie() {
  blink(LED, 2, 2000);
}


/*******************************************************************/
/** STATE HANDLERS *************************************************/
/*******************************************************************/

// when you're offline
void while_OFFLINE() {
  setLED(LED, 0);
}

// while connecting to XMPP
void while_CONNECTING() {
  setLED(LED, 5);
}

// while online, waiting for a game to start
void while_ONLINE() {
  setLED(LED, 50);
}

// while waiting for you to say you're ready
void while_WAITING_FOR_YOUR_READY() {
  setLED(LED, 255);
}

// while waiting for the other player to say they're ready
void while_WAITING_FOR_THEIR_READY() {
  setLED(LED, 255);
}

// while waiting for both players to choose a weapon
void while_READY_TO_PLAY() {
  setLED(LED, 255);
}

// while waiting for you to choose a weapon
void while_WAITING_FOR_YOUR_CHOICE() {
  setLED(LED, 255);
}

// while waiting for the other player to choose a weapon
void while_WAITING_FOR_THEIR_CHOICE() {
  setLED(LED, 255);
}

// while waiting to find out who won or lost
void while_WAITING_FOR_RESULT() {
  setLED(LED, 255);
}

/*******************************************************************/
/** SETUP AND MAIN LOOP ********************************************/
/*******************************************************************/

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(SERIAL_BAUDRATE);

  // wire up your event handlers here (all optional)
  rpa.remote_ready = *on_remote_ready;
  rpa.they_choose = *on_they_choose;
  rpa.you_win = *on_you_win;
  rpa.you_lose = *on_you_lose;
  rpa.tie = *on_tie;

  // wire up your state entry handlers here (all optional)
  rpa.OFFLINE = *while_OFFLINE;
  rpa.CONNECTING = *while_CONNECTING;
  rpa.ONLINE = *while_ONLINE;
  rpa.WAITING_FOR_YOUR_READY = *while_WAITING_FOR_YOUR_READY;
  rpa.WAITING_FOR_THEIR_READY = *while_WAITING_FOR_THEIR_READY;
  rpa.READY_TO_PLAY = *while_READY_TO_PLAY;
  rpa.WAITING_FOR_YOUR_CHOICE = *while_WAITING_FOR_YOUR_CHOICE;
  rpa.WAITING_FOR_THEIR_CHOICE = *while_WAITING_FOR_THEIR_CHOICE;
  rpa.WAITING_FOR_RESULT = *while_WAITING_FOR_RESULT;

  // tell Node that the Arduino is ready and wants it to connect to XMPP
  rpa.say_hello();
}

void loop() {
  rpa.check_input_from_serial();
}

/*******************************************************************/
/** HELPER FUNCTIONS ***********************************************/
/*******************************************************************/

// does any necessary transformation from a LED value to the value that should be written to the pin (using analogWrite)
byte convertRGBval(byte val) {
  return val;
}

// immediaely set the given led to the given brightness
void setLED(byte pin, byte val) {
  analogWrite(pin, convertRGBval(val));
}

void blink(byte pin, byte times, int delayTime) {
  for (int i = 0; i < times; i++) {
    setLED(pin, 255);
    delay(delayTime);
    setLED(pin, 0);
    delay(delayTime / 2);
  }
}





