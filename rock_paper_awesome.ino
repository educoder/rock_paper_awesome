#include <stdio.h>

#define SERIAL_BAUDRATE 9600
#define MILLISECONDS_PER_FRAME 30

// the pins that control the brightness of RED, GREEN, and BLUE LEDs respectively
#define R 9
#define G 10
#define B 11

// pins that receive input for ROCK, PAPER, and SCISSOR
#define PIN_ROCK 3
#define PIN_PAPER 4
#define PIN_SCISSORS 5

// PROTOCOL
// >> incoming messages from node over serialport
#define PROT_ONLINE '@'  // you are now online
#define PROT_OFFLINE '#' // you are now offline
#define PROT_NEW_GAME 'N'  // sent by node to indicate that we're starting a new game
#define PROT_THEY_CHOSE_ROCK 'R'
#define PROT_THEY_CHOSE_PAPER 'P'
#define PROT_THEY_CHOSE_SCISSORS 'S'
#define PROT_WAITING_FOR_YOU ':'
#define PROT_WAITING_FOR_THEM ';'
#define PROT_YOU_WON '+'
#define PROT_YOU_LOST '-'
#define PROT_TIE '='
// << outgoing messages to node over serialport
#define PROT_ARDUINO_READY '~' // sent to daemon to signal that the arduino is ready
#define PROT_YOU_CHOSE_ROCK 'r'
#define PROT_YOU_CHOSE_PAPER 'p'
#define PROT_YOU_CHOSE_SCISSORS 's'
#define PROT_READY_FOR_NEW_GAME 'n' // sent to daemon to signal that the arduino is ready for a new game

// states
#define STATE_OFFLINE 0
#define STATE_WAITING_FOR_EITHER_CHOICE 1
#define STATE_WAITING_FOR_YOUR_CHOICE 2
#define STATE_WAITING_FOR_THEIR_CHOICE 3
#define STATE_WAITING_FOR_GAME_OVER_CONFIRMATION 4

// LED modes
#define LED_MODE_STEADY 0
#define LED_MODE_FADE 1
#define LED_MODE_BLINK 2
#define LED_MODE_PULSATE 3

char state = STATE_OFFLINE;
char yourChoice = NULL;
char theirChoice = NULL;

// set to true once the choice has been reset -- used to prevent double-triggering when button is held down
bool choiceWasReset = false;

long frameCount = 0;

byte cR, cG, cB; // current values of the red, green, and blue LEDs
byte tR, tG, tB; // target values of the LEDs (when fading/blinking/pulsating)
int rR, rG, rB = 0; // rate at which the LEDs should change per frame (has different meaning for fading, pulsating, and blinking)
byte mR, mG, mB = LED_MODE_STEADY; // the current mode for each LED (steady, fading, blinking, pulsating, etc.)

void setup() {
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
  Serial.print(PROT_ARDUINO_READY); // tell the daemon that we're ready
}

void loop() {
  if (state == STATE_WAITING_FOR_EITHER_CHOICE || state == STATE_WAITING_FOR_YOUR_CHOICE) {
    if (digitalRead(PIN_ROCK) == HIGH)
      youChose(PROT_YOU_CHOSE_ROCK);
    else if (digitalRead(PIN_PAPER) == HIGH)
      youChose(PROT_YOU_CHOSE_PAPER);
    else if (digitalRead(PIN_SCISSORS) == HIGH)
      youChose(PROT_YOU_CHOSE_SCISSORS);
  } else if (state = STATE_WAITING_FOR_GAME_OVER_CONFIRMATION) {
    if (digitalRead(PIN_ROCK) == LOW && yourChoice == PROT_YOU_CHOSE_ROCK
        || digitalRead(PIN_PAPER) == LOW && yourChoice == PROT_YOU_CHOSE_PAPER
        || digitalRead(PIN_SCISSORS) == LOW && yourChoice == PROT_YOU_CHOSE_SCISSORS) {
      choiceWasReset = true;
    }

    if (choiceWasReset && (digitalRead(PIN_ROCK) == HIGH || digitalRead(PIN_PAPER) == HIGH || digitalRead(PIN_SCISSORS))) {
      choiceWasReset = false;
      readyForNewGame();
    }
  }
  
  if (Serial.available() > 0) {
    in(Serial.read());
  }
}

// called for each incoming character from the serialport
void in(char c) {
  switch (c) {
    case PROT_ONLINE:
      online();
      break;
    case PROT_OFFLINE:
      offline();
      break;
    case PROT_THEY_CHOSE_ROCK:
    case PROT_THEY_CHOSE_PAPER:
    case PROT_THEY_CHOSE_SCISSORS:
      theyChose(c);
      break;
    case PROT_WAITING_FOR_YOU:
      waitingForYou();
      break;
    case PROT_WAITING_FOR_THEM:
      waitingForThem();
      break;
    case PROT_YOU_WON:
      youWin();
      break;
    case PROT_YOU_LOST:
      youLose();
      break;
    case PROT_TIE:
      tie();
      break;
    case PROT_NEW_GAME:
      newGame();
      break;
  }
}

// called when online
void online() {
  state = STATE_WAITING_FOR_EITHER_CHOICE;
  blink(G, 3, 1000);
}

// called when offline
void offline() {
  state = STATE_OFFLINE;
  blink(R, 3, 1000);
}

// called when you chose rock, paper, or scissors
void youChose(char choice) {
  yourChoice = choice;
  switch(choice) {
    case PROT_YOU_CHOSE_ROCK:
      setLED(R, 255);
      break;
    case PROT_YOU_CHOSE_PAPER:
      setLED(G, 255);
      break;
    case PROT_YOU_CHOSE_SCISSORS:
      setLED(B, 255);
      break;
  }
  Serial.print(choice);
}

// called when they chose rock, paper, or scissors
void theyChose(char choice) {
  theirChoice = choice;
}

// called when waiting for you to make a choice
void waitingForYou() {
  blink(G, 5, 200);
}

// called when waiting for them to make a choice
void waitingForThem() {
  blink(B, 5, 200);
}

void youWin() {
  blink(G, 5, 1000);
}

void youLose() {
  state = STATE_WAITING_FOR_GAME_OVER_CONFIRMATION;
  blink(R, 5, 1000);
}

void tie() {
  state = STATE_WAITING_FOR_GAME_OVER_CONFIRMATION;
  blink(B, 5, 1000);
}

void readyForNewGame() {
  Serial.print(PROT_READY_FOR_NEW_GAME);
}

void newGame() {
  theirChoice = NULL;
  yourChoice = NULL;
  state = STATE_WAITING_FOR_EITHER_CHOICE;
  
  // TODO: do some blinky stuff here

  //delay(2000); // wait 2 seconds before listening for other signals... otherwise we might catch a button held down too long
}

// does any necessary transformation from a LED value to the value that should be written to the pin (using analogWrite)
byte convertRGBval(byte val) {
  // due to the way my LEDs are wired, my pin values are inverted, so for me 0 == maximum, 255 == minimum
  // get rid of the "255-" for yours to revert to normal usage
  //return 255-val;
  return val;
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
  for (int i = 0; i <= times; i++) {
    setLED(pin, 255);
    delay(delayTime);
    setLED(pin, 0);
    delay(delayTime / 2);
  }
}


