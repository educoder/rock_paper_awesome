#include <stdio.h>
#include "pitches.h"
#include <Servo.h> 

#define SERIAL_BAUDRATE 9600
#define MILLISECONDS_PER_FRAME 30

// ****** Sounds for winning a round ******
// notes for the win melody (for winning lab song):
int melodyWin[] = {
  NOTE_G4, NOTE_G4, NOTE_B4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_E5, NOTE_D5, NOTE_CS5, NOTE_D5, NOTE_G4, 
NOTE_G4, NOTE_B4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_E5, NOTE_D5, NOTE_B4, NOTE_G4};
  
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurationsWin[] = {
  8, 4, 4, 2, 8, 8, 8, 8, 8, 2, 8, 4, 4, 2, 8, 8, 8, 8, 8, 2};
// end of winning sounds

// ****** Sounds for losing a round ******
// notes for the losing melody:
int melodyLose[] = {
NOTE_C8, NOTE_B7, NOTE_AS7, NOTE_A7, NOTE_C8, NOTE_GS7, NOTE_GS7, NOTE_G7, NOTE_FS7, NOTE_F7, NOTE_E7, NOTE_DS7, NOTE_D7, NOTE_CS7, NOTE_C7, NOTE_B6, NOTE_AS6, NOTE_A6, NOTE_GS6, NOTE_G6, NOTE_FS6, NOTE_F6, NOTE_E6, NOTE_DS6, NOTE_D6, NOTE_CS6, NOTE_C6, NOTE_B5, NOTE_AS5, NOTE_A5, NOTE_GS5, NOTE_G5, NOTE_FS5, NOTE_F5, NOTE_E5, NOTE_DS5, NOTE_D5, NOTE_CS5, NOTE_C5, NOTE_B4, NOTE_AS4, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_FS4, NOTE_F4, NOTE_E4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_C4, NOTE_B3, NOTE_AS3, NOTE_A3, 
NOTE_GS3, NOTE_G3, NOTE_FS3, NOTE_F3, NOTE_E3, NOTE_DS3   
};

// note durations: 16 = 32nd note, 8 = sixteenth note, 4 = quarter note, 2 = half note, 1 = whole note etc.:
int noteDurationsLose[] = {
  32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 
32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 4, 4, 1};

// end of losing sounds

// the pins that control the brightness of RED, GREEN, and BLUE LEDs respectively
#define R 9
#define G 10
#define B 11
#define Y 12

// pins that receive input for ROCK, PAPER, and SCISSOR
#define PIN_ROCK 3
#define PIN_PAPER 4
#define PIN_SCISSORS 5

// create servo object to control a servo 
Servo myservo;
int servoPos = 0;    // variable to store the servo position

// defining the seonsor pin to analog pin 0
int sensorPin = 0;  

// pin for new game button
#define PIN_READY_NEW_GAME 2

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
#define STATE_WAITING_FOR_READY 1
#define STATE_WAITING_FOR_EITHER_CHOICE 2
#define STATE_WAITING_FOR_YOUR_CHOICE 3
#define STATE_WAITING_FOR_THEIR_CHOICE 4
#define STATE_WAITING_FOR_GAME_OVER_CONFIRMATION 5

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
  pinMode(Y, OUTPUT);
  digitalWrite(PIN_ROCK, LOW);
  digitalWrite(PIN_PAPER, LOW);
  digitalWrite(PIN_SCISSORS, LOW);
  digitalWrite(PIN_READY_NEW_GAME, LOW);
  pinMode(PIN_ROCK, INPUT);
  pinMode(PIN_PAPER, INPUT);
  pinMode(PIN_SCISSORS, INPUT);
  pinMode(PIN_READY_NEW_GAME, INPUT);
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object 
  myservo.write(7);
  setRGB(0,0,0);
  Serial.begin(SERIAL_BAUDRATE);
  Serial.print(PROT_ARDUINO_READY); // tell the daemon that we're ready
}

int readSensorValue() {
 int val = analogRead(sensorPin); 
 Serial.println(val);
 return val;
}

void loop() {
  if (state == STATE_WAITING_FOR_EITHER_CHOICE || state == STATE_WAITING_FOR_YOUR_CHOICE) {
    if (readSensorValue() > 300){
        for (int sensorCheck = 0; sensorCheck < 5 && (readSensorValue()  > 300) ; sensorCheck++){
          Serial.println(sensorCheck);
          if (sensorCheck == 4){
            youChose(PROT_YOU_CHOSE_ROCK);
            }
           delay(1000);         
          }
    } 
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
  
  if (digitalRead(PIN_READY_NEW_GAME) == HIGH)
    readyForNewGame();
    
  
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
  state = STATE_WAITING_FOR_READY;
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
  state = STATE_WAITING_FOR_THEIR_CHOICE;
  Serial.print(choice);
}

// called when they chose rock, paper, or scissors
void theyChose(char choice) {
  theirChoice = choice;
  blink(Y, 5, 200);
}

void think() {
  switch(random(1,3)) {
  case 1:
    youChose(PROT_YOU_CHOSE_ROCK);
    break;
  case 2:
    youChose(PROT_YOU_CHOSE_PAPER);
    break;
  case 3:
    youChose(PROT_YOU_CHOSE_SCISSORS);
    break;
  }
}

// called when waiting for you to make a choice
void waitingForYou() {
  myservo.write(45);
  for(servoPos = 45; servoPos < 135; servoPos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  for(servoPos = 135; servoPos>=45; servoPos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  blink(G, 5, 200);
}

// called when waiting for them to make a choice
void waitingForThem() {
  blink(B, 5, 200);
}

void youWin() {
  //blink(G, 5, 1000);
  myservo.write(59);
  delay(500);
  myservo.write(119);
  delay(500);
  myservo.write(59);
  delay(500);
  myservo.write(119);
  delay(500);
  myservo.write(79);
  for (int thisNote = 0; thisNote < 20; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDurationWin = 1000/noteDurationsWin[thisNote];
    tone(8, melodyWin[thisNote],noteDurationWin);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDurationWin * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

void youLose() {
  state = STATE_WAITING_FOR_GAME_OVER_CONFIRMATION;
  myservo.write(59);
  delay(500);
  myservo.write(119);
  delay(500);
  myservo.write(59);
  delay(500);
  myservo.write(119);
  delay(500);
  myservo.write(155);
  for (int thisNote = 0; thisNote < 60; thisNote++) {
  
    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDurationLose = 1000/noteDurationsLose[thisNote];
    tone(8, melodyLose[thisNote],noteDurationLose);
  
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDurationLose * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

void tie() {
  state = STATE_WAITING_FOR_GAME_OVER_CONFIRMATION;
  myservo.write(59);
  delay(500);
  myservo.write(119);
  delay(500);
  myservo.write(59);
  delay(500);
  myservo.write(119);
  delay(500);
  myservo.write(44);
  blink(B, 5, 1000);
}

void readyForNewGame() {
  Serial.print(PROT_READY_FOR_NEW_GAME);
  myservo.write(89);
}

void newGame() {
  theirChoice = NULL;
  yourChoice = NULL;
  myservo.write(7);
  state = STATE_WAITING_FOR_EITHER_CHOICE;

  blink(G, 5, 100);
  // TODO: do some blinky stuff here

  //delay(2000); // wait 2 seconds before listening for other signals... otherwise we might catch a button held down too long
}

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
  for (int i = 0; i <= times; i++) {
    setLED(pin, 255);
    delay(delayTime);
    setLED(pin, 0);
    delay(delayTime / 2);
  }
}

