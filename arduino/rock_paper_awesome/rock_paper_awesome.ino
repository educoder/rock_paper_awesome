#include <stdio.h>
#include "Button.h"
#include "RPA.h"

#define SERIAL_BAUDRATE 9600

// the pins that control the brightness of RED, GREEN, and BLUE LEDs respectively
#define R 9
#define G 10
#define B 11


// instantiate a RockPaperAwesome state machine
RPA rpa = RPA();


/*******************************************************************/
/** EVENT HANDLERS *************************************************/
/*******************************************************************/

// on_connected event handler -- called when we're connected to the XMPP chatroom
void connected() {
  //blink(G, 3, 1000);
}

// on_ready event handler -- called when we've signaled that we're ready to play a game
void ready() {
  //blink(G, 3, 1000);
  setLED(G, 15);
}

// on_you_choose event handler - called when you have chosen a weapon
void you_chose(RPA::WEAPON weapon) {
  switch(weapon) {
    case RPA::ROCK:
      setLED(R, 255);
      break;
    case RPA::PAPER:
      setLED(G, 255);
      break;
    case RPA::SCISSORS:
      setLED(B, 255);
      break;
  }
}

// on_you_win event handler - called when you have won
void you_win() {
  blink(G, 5, 600);
}

// on_you_lose event handler - called when you have lost
void you_lose() {
  blink(R, 5, 600);
}

// on_tie event handler - called when you have tied
void tie() {
  blink(B, 5, 600);
}

// NOTE: there are many more possible handlers -- one for each event


// on_enter_state handler - called when the rpa enters a new state
void entered_state(RPA::States::STATE state) {
  //blink(B, 1, 200);
  switch (state) {

    case RPA::States::WAITING_FOR_YOUR_READY:
      rpa.ready();
      break;

    case RPA::States::WAITING_FOR_YOUR_CHOICE:
      randomly_choose_weapon();
      break;

    case RPA::States::OFFLINE:
      break;

    // you can add a case for each possible state

  }
}

// on_exit_state handler - called when the rpa exits a state
void left_state(RPA::States::STATE old_state, RPA::States::STATE new_state) {
  switch (old_state) {

    // you can add a case for each possible state
    // NOTE: don't try to trigger events here or do anything
    //       else that results in an event transition -- at
    //       this point the previous transition hasn't completed
    //       and you will either derail it or cause an invalid_transition_error 

  }
}

/*******************************************************************/
/** SETUP AND MAIN LOOP ********************************************/
/*******************************************************************/

void setup() {
  // set up the Arduino hardware
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  // digitalWrite(PIN_ROCK, LOW);
  // digitalWrite(PIN_PAPER, LOW);
  // digitalWrite(PIN_SCISSORS, LOW);
  // pinMode(PIN_ROCK, INPUT);
  // pinMode(PIN_PAPER, INPUT);
  // pinMode(PIN_SCISSORS, INPUT);
  setRGB(0,0,0);
  Serial.begin(SERIAL_BAUDRATE);

  // wire up your event handlers here (all optional)
  //rpa.on_connect = *_____;
  rpa.on_connected = *connected;
  rpa.on_ready = *ready;
  //rpa.on_remote_ready = *_____;
  //rpa.on_offline = *_____;
  rpa.on_you_choose = *you_chose;
  rpa.on_you_win = *you_win;
  rpa.on_you_lose = *you_lose;
  rpa.on_tie = *tie;
  
  // .. and  state enter/exit handlers here (all optional)
  rpa.on_enter_state = *entered_state;
  rpa.on_exit_state = *left_state;

  // tell Node that the arduino is ready and wants it to connect to XMPP
  rpa.connect();
}

void loop() {
  rpa.check_input_from_serial();
}

/*******************************************************************/
/** HELPER FUNCTIONS ***********************************************/
/*******************************************************************/

void randomly_choose_weapon() {
  int choice = random(1,4);
  if (choice == 1)
      rpa.you_choose(RPA::ROCK);
  else if (choice == 2)
      rpa.you_choose(RPA::PAPER);
  else if (choice == 3)
      rpa.you_choose(RPA::SCISSORS);
}

// does any necessary transformation from a LED value to the value that should be written to the pin (using analogWrite)
byte convertRGBval(byte val) {
  // due to the way my LEDs are wired, my pin values are inverted, so for me 0 == maximum, 255 == minimum
  // get rid of the "255-" for yours to revert to normal usage
  return val;
  //return val;
}

// immediaely set the given led to the given brightness
void setLED(byte pin, byte val) {
  analogWrite(pin, convertRGBval(val));
}

// immediatley set the RED, GREEN, and BLUE leds to the given brightness
void setRGB(byte r, byte g, byte b) {
  setLED(R, r); 
  setLED(G, g); 
  setLED(B, b);
}

void blink(byte pin, byte times, int delayTime) {
  for (int i = 0; i < times; i++) {
    setLED(pin, 255);
    delay(delayTime);
    setLED(pin, 0);
    delay(delayTime / 2);
  }
}





