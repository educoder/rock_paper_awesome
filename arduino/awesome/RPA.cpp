#include "RPA.h" 

RPA::RPA() {
    inputBuffer = "";
}

void RPA::signal_ready() {
    out("~");
}

void RPA::present() {
    out("< present");
}

void RPA::absent() {
    out("< absent");
}

void RPA::choose_ROCK() { 
    out("< you_choose Rock");
}

void RPA::choose_PAPER() {
    out("< you_choose Paper");
}

void RPA::choose_SCISSORS() {
    out("< you_choose Scissors");
}

void RPA::check_input_from_serial() {
    char c;
    if (Serial.available() > 0) {
        c = Serial.read();
        if (c == '\n') {
            in(inputBuffer);
            inputBuffer = "";
        } else {
            inputBuffer.concat(c);
            // FIXME: what if we exceed inputBuffer's size?
        }
    }
}

// processes an incoming command line from Serial
void RPA::in(String &str) {
    char input[255];
    str.toCharArray(input, 255);
    char *p = input;
    String ctrl = strtok_r(p, " ", &p);
    String ev = strtok_r(p, " ", &p);
    String arg = strtok_r(p, " ", &p);
    
    if (ctrl == ">") {
        // we have an incoming event
        if (ev == "you_are_present") {
            when_you_are_present();
        } else if (ev == "you_are_absent") {
            when_you_are_absent();
        } else if (ev == "opponent_is_present") {
            when_opponent_is_present(arg); // arg is the name/id of the opponent
        } else if (ev == "opponent_is_absent") {
            when_opponent_is_absent(arg); // arg is the name/id of the opponent
        } else if (ev == "you_choose_weapon") {
            when_you_choose_weapon((RPA::WEAPON) arg[0]); // arg is the weapon
        } else if (ev == "opponent_chooses_weapon") {
            when_opponent_chooses_weapon((RPA::WEAPON) arg[0]); // arg is the weapon
        } else if (ev == "you_win") {
            when_you_win();
        } else if (ev == "you_lose") {
            when_you_lose();
        } else if (ev == "tie") {
            when_tie();
        } else if (ev == "error") {
            when_error();
        }
    }

    p = NULL;
}


// sends out a command line over Serial
void RPA::out(const char *str) {
    Serial.println(str);
}


// sends out a command line over Serial
void RPA::out(String &str) {
    Serial.println(str);
}