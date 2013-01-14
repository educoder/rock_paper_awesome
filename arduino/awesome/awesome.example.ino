#include "Button.h"
#include "RPA.h"

#define SERIAL_BAUDRATE 9600

// the pins that control the brightness of RED, GREEN, and BLUE LEDs respectively
#define R 9
#define G 10
#define B 11

// pins that receive input for ROCK, PAPER, and SCISSOR
#define PIN_ROCK 5
#define PIN_PAPER 7
#define PIN_SCISSORS 6
// pin that receives input from the READY button
#define PIN_READY 3

// wait this many ms before considering that a button has been pressed again
#define BUTTON_DEBOUNCE_WAIT 500

// instantiate a RockPaperAwesome state machine
RPA rpa = RPA();

// Button wrappers; uses the Button library
//  -- see: https://github.com/virgildisgr4ce/Button
Button rockButton = Button(PIN_ROCK, BUTTON_PULLUP_INTERNAL, true, BUTTON_DEBOUNCE_WAIT);
Button paperButton = Button(PIN_PAPER, BUTTON_PULLUP_INTERNAL, true, BUTTON_DEBOUNCE_WAIT);
Button scissorsButton = Button(PIN_SCISSORS, BUTTON_PULLUP_INTERNAL, true, BUTTON_DEBOUNCE_WAIT);

Button readyButton = Button(PIN_READY, BUTTON_PULLUP_INTERNAL, true, BUTTON_DEBOUNCE_WAIT);

bool present = false;

/*******************************************************************/
/** EVENT HANDLERS *************************************************/
/*******************************************************************/

// triggered when you're present and ready to play a game
void when_you_are_present() {
  //blink(G, 3, 1000);
  setLED(G, 50);
}

// triggered when you go away and are no longer able play the game
void when_you_are_absent() {
  //blink(G, 3, 1000);
  setLED(G, 5);
}

// triggered when the other player is present and ready to play a game
void when_opponent_is_present(String opponentName) {
  //blink(G, 3, 1000);
  setLED(R, 50);
}

// triggered when the other player goes away and is no longer able to play a game
void when_opponent_is_absent(String opponentName) {
  //blink(G, 3, 1000);
  setLED(G, 5);
}

// triggered when you have chosen a weapon
void when_you_choose_weapon(RPA::WEAPON weapon) {
  setRGB(0,0,0);
  if (weapon == RPA::ROCK)
    setLED(R, 255);
  else if (weapon == RPA::PAPER)
      setLED(G, 255);
  else if (weapon == RPA::SCISSORS)
      setLED(B, 255);
}

// triggered when another player has chosen a weapon
void when_opponent_chooses_weapon(RPA::WEAPON weapon) {
  //blink(G, 3, 1000);
}

// triggered when you've won
void when_you_win() {
  setRGB(0,0,0);
  blink(G, 5, 600);
}

// triggered when you've lost
void when_you_lose() {
  setRGB(0,0,0);
  blink(R, 5, 600);
}

// triggered when you've tied
void when_tie() {
  setRGB(0,0,0);
  blink(B, 5, 600);
}

// triggered when some sort of serious error occurs (usually requiring a game reset)
void when_error() {
  blink(R, 5, 600);
}

/*******************************************************************/
/** SETUP AND MAIN LOOP ********************************************/
/*******************************************************************/

void foo() { Serial.println("overridden foo"); };

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

  Serial.begin(SERIAL_BAUDRATE);
  setRGB(0,0,0);

  // tell awesome.js that the Arduino is ready
  rpa.signal_ready();
  blink(G, 2, 200);
}

void loop() {
  if (readyButton.uniquePress()) {
    if (present) {
      present = false;
      rpa.absent();
    } else {
      present = true;
      rpa.present();
    }
  }
  
  if (rockButton.uniquePress())
    rpa.choose_ROCK();
  
  if (paperButton.uniquePress())
    rpa.choose_PAPER();

  if (scissorsButton.uniquePress())
    rpa.choose_SCISSORS();

  // must call this to process data from awesome.js
  rpa.check_input_from_serial();
}

/***********************************************************************/
/** LED HELPER FUNCTIONS ***********************************************/
/***********************************************************************/

// does any necessary transformation from a LED value to the value that should be written to the pin (using analogWrite)
byte convertRGBval(byte val) {
  // due to the way my LEDs are wired, my pin values are inverted, so for me 0 == maximum, 255 == minimum
  // get rid of the "255-" for yours to revert to normal usage
  //return val;
  return 255-val;
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

void blink(byte pin, byte times, int delayTime, int maxBrightness) {
  for (int i = 0; i < times; i++) {
    setLED(pin, maxBrightness);
    delay(delayTime);
    setLED(pin, 0);
    delay(delayTime / 2);
  }
}

void blink(byte pin, byte times, int delayTime) {
  blink(pin, times, delayTime, 255);
}
