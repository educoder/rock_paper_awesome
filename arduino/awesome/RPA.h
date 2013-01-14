#include <stdio.h>
#include "Arduino.h"

class RPA {
    public:
        RPA();

        enum WEAPON { ROCK = 'R', PAPER = 'P', SCISSORS = 'S' };

        void signal_ready(void);

        void present(void);
        void absent(void);
        void choose_ROCK(void);
        void choose_PAPER(void);
        void choose_SCISSORS(void);

        void check_input_from_serial(void);

    protected:

        String inputBuffer;

        String state;

        // processes an incoming command line from Serial
        void in(String &str);

        // sends out a command line over Serial
        void out(const char *str);
        void out(String &str);

};

// POSSIBLE EVENTS
void when_you_are_present(void);
void when_you_are_absent(void);
void when_opponent_is_present(String opponentName);
void when_opponent_is_absent(String opponentName);
void when_you_choose_weapon(RPA::WEAPON weapon);
void when_opponent_chooses_weapon(RPA::WEAPON weapon);
void when_you_win(void);
void when_you_lose(void);
void when_tie(void);
void when_error(void);