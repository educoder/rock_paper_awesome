#include <stdio.h>
#include "RPA.h"

#define SERIAL_BAUDRATE 9600

// the pins that control the brightness of RED, GREEN, and BLUE LEDs respectively
#define R 9
#define G 10
#define B 11

// pins that receive input for ROCK, PAPER, and SCISSOR
#define PIN_ROCK 3
#define PIN_PAPER 4
#define PIN_SCISSORS 5

// pin that receives input from the NEW GAME (READY) button
#define PIN_READY 6

// LED modes
#define LED_MODE_STEADY 0
#define LED_MODE_FADE 1
#define LED_MODE_BLINK 2
#define LED_MODE_PULSATE 3

// instantiate a RockPaperAwesome state machine
RPA rpa = RPA();

/*******************************************************************/
/** EVENT HANDLERS *************************************************/
/*******************************************************************/

void online() {
  //blink(G, 3, 1000);
  setLED(G, 5);
}

void ready() {
  //blink(G, 3, 1000);
  setLED(G, 15);
}

// on_enter_state handler - called when the rpa enters a new state
void entered_state(RPA::States::STATE state) {
  blink(B, state, 200);
  switch (state) {

    case RPA::States::OFFLINE:
      blink(R, 3, 600);
      break;

    case RPA::States::WAITING_FOR_EITHER_READY:
    case RPA::States::WAITING_FOR_YOUR_READY:
      rpa.ready();
      break;

    case RPA::States::WAITING_FOR_YOUR_CHOICE:
      choose_random_weapon();
      break;

  }
}

// on_exit_state handler - called when the rpa exits a state
void left_state(RPA::States::STATE old_state, RPA::States::STATE new_state) {
  switch (old_state) {

  }
}

// on_you_choose handler - called when you have chosen a weapon
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

// on_you_win handler - called when you have won
void you_win() {
  blink(G, 5, 600);
}

// on_you_lose handler - called when you have lost
void you_lose() {
  blink(R, 5, 600);
}

// on_tie handler - called when you have tied
void tie() {
  blink(B, 5, 600);
}

void choose_random_weapon() {
  RPA::WEAPON choice;
  switch(random(1,3)) {
  case 1:
    choice = RPA::ROCK;
    break;
  case 2:
    choice = RPA::PAPER;
    break;
  case 3:
    choice = RPA::SCISSORS;
    break;
  }

  rpa.you_choose(choice);
}

/*******************************************************************/
/** SETUP AND MAIN LOOP ********************************************/
/*******************************************************************/

void setup() {
  // initialize pins
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  digitalWrite(PIN_ROCK, LOW);
  digitalWrite(PIN_PAPER, LOW);
  digitalWrite(PIN_SCISSORS, LOW);
  pinMode(PIN_ROCK, INPUT);
  pinMode(PIN_PAPER, INPUT);
  pinMode(PIN_SCISSORS, INPUT);
  setRGB(0,0,0);
  Serial.begin(SERIAL_BAUDRATE);

  // set up your handlers here
  rpa.on_online = *online;
  rpa.on_ready = *ready;
  
  rpa.on_enter_state = *entered_state;
  rpa.on_exit_state = *left_state;

  rpa.on_you_choose = *you_chose;
  rpa.on_you_win = *you_win;
  rpa.on_you_lose = *you_lose;
  rpa.on_tie = *tie;

  // tell Node that the arduino is ready and wants it to connect to XMPP
  rpa.connect();
}

void loop() {
  if (digitalRead(PIN_READY) == HIGH) {
    rpa.ready();
  } else if (digitalRead(PIN_ROCK) == HIGH) {
    rpa.you_choose(RPA::ROCK);
  } else if (digitalRead(PIN_PAPER) == HIGH) {
    rpa.you_choose(RPA::PAPER);
  } else if (digitalRead(PIN_SCISSORS) == HIGH) {
    rpa.you_choose(RPA::SCISSORS);
  }

  rpa.check_input_from_serial();
}

/*******************************************************************/
/** HELPER FUNCTIONS ***********************************************/
/*******************************************************************/

// does any necessary transformation from a LED value to the value that should be written to the pin (using analogWrite)
byte convertRGBval(byte val) {
  // due to the way my LEDs are wired, my pin values are inverted, so for me 0 == maximum, 255 == minimum
  // get rid of the "255-" for yours to revert to normal usage
  return 255-val;
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





