#include "RPA.h" 


RPA::RPA() {
    inputBuffer = "";
}

void RPA::say_hello() {
    out("~");
}

void RPA::bow() {
    out("< ready");
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
    String cmd = strtok_r(p, " ", &p);
    String arg = strtok_r(p, " ", &p);
    
    if (ctrl == ">") {
        // we have an incoming event
        if (cmd == "connect" && *connect)
            connect();
        else if (cmd == "connected" && *connected)
            connected();
        else if (cmd == "remote_ready" && *remote_ready)
            remote_ready();
        else if (cmd == "ready" && *ready)
            ready();
        else if (cmd == "you_choose" && *you_choose)
            you_choose(arg);
        else if (cmd == "they_choose" && *they_choose)
            they_choose(arg);
        else if (cmd == "you_win" && *you_win)
            you_win();
        else if (cmd == "you_lose" && *you_lose)
            you_lose();
        else if (cmd == "tie" && *tie)
            tie();
        else if (cmd == "disconnected" && *disconnected)
            disconnected();

    } else if (ctrl == "=") {
        // we have an incoming state update
        if (cmd == "OFFLINE" && *OFFLINE)
            OFFLINE();
        else if (cmd == "CONNECTING" && *CONNECTING)
            CONNECTING();
        else if (cmd == "ONLINE" && *ONLINE)
            ONLINE();
        else if (cmd == "WAITING_FOR_YOUR_READY" && *WAITING_FOR_YOUR_READY)
            WAITING_FOR_YOUR_READY();
        else if (cmd == "WAITING_FOR_THEIR_READY" && *WAITING_FOR_THEIR_READY)
            WAITING_FOR_THEIR_READY();
        else if (cmd == "READY_TO_PLAY" && *READY_TO_PLAY)
            READY_TO_PLAY();
        else if (cmd == "WAITING_FOR_THEIR_CHOICE" && *WAITING_FOR_THEIR_CHOICE)
            WAITING_FOR_THEIR_CHOICE();
        else if (cmd == "WAITING_FOR_YOUR_CHOICE" && *WAITING_FOR_YOUR_CHOICE)
            WAITING_FOR_YOUR_CHOICE();
        else if (cmd == "WAITING_FOR_RESULT" && *WAITING_FOR_RESULT)
            WAITING_FOR_RESULT();

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