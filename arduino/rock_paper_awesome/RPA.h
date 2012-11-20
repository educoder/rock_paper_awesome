#include <stdio.h>
#include "Arduino.h"

class RPA {
    public:
        RPA();

        enum WEAPON { ROCK = 'R', PAPER = 'P', SCISSORS = 'S' };

        void say_hello();

        void bow();
        void choose_ROCK();
        void choose_PAPER();
        void choose_SCISSORS();

        // EVENTS
        void (*connect)();
        void (*connected)();
        void (*ready)();
        void (*remote_ready)();
        void (*you_choose)(String weapon);
        void (*they_choose)(String weapon);
        void (*you_win)();
        void (*you_lose)();
        void (*tie)();
        void (*disconnected)();

        // STATES
        void (*OFFLINE)();
        void (*CONNECTING)();
        void (*ONLINE)();
        void (*WAITING_FOR_YOUR_READY)();
        void (*WAITING_FOR_THEIR_READY)();
        void (*READY_TO_PLAY)();
        void (*WAITING_FOR_YOUR_CHOICE)();
        void (*WAITING_FOR_THEIR_CHOICE)();
        void (*WAITING_FOR_RESULT)();

        void check_input_from_serial();

    protected:

        String inputBuffer;

        String state;

        RPA::WEAPON your_weapon;
        RPA::WEAPON their_weapon;

        // processes an incoming command line from Serial
        void in(String &str);

        // sends out a command line over Serial
        void out(const char *str);
        void out(String &str);


};