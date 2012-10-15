#include <stdio.h>
#include "Arduino.h"

class RPA {
    public:
        RPA();

        enum WEAPON { ROCK = 'R', PAPER = 'P', SCISSORS = 'S' };

        struct States {
            enum STATE {
                OFFLINE = 0,
                CONNECTING = 1,
                ONLINE = 2,
                WAITING_FOR_THEIR_READY = 3,
                WAITING_FOR_YOUR_READY = 4,
                READY_TO_PLAY = 5,
                WAITING_FOR_THEIR_CHOICE = 6,
                WAITING_FOR_YOUR_CHOICE = 7,
                WAITING_FOR_RESULT = 8
            };
        };

        struct Protocol {
            public: 
                // << incoming messages from node over serialport
                static const char ONLINE = '@';  // you are now online
                static const char OFFLINE = '#'; // you are now offline
                static const char REMOTE_READY = 'N';  // sent by node to indicate that we're starting a new game
                static const char THEY_CHOSE_ROCK = 'R';
                static const char THEY_CHOSE_PAPER = 'P';
                static const char THEY_CHOSE_SCISSORS = 'S';
                static const char WAITING_FOR_YOU = ':';
                static const char WAITING_FOR_THEM = ';';
                static const char YOU_WON = '+';
                static const char YOU_LOST = '-';
                static const char TIE = '=';
                // >> outgoing messages to node over serialport
                static const char ARDUINO_ON = '~'; // sent to daemon to signal that the arduino is ready
                static const char YOU_CHOSE_ROCK = 'r';
                static const char YOU_CHOSE_PAPER = 'p';
                static const char YOU_CHOSE_SCISSORS = 's';
                static const char READY_FOR_NEW_GAME = 'n'; // sent to daemon to signal that the arduino is ready for a new game

                // NOTE: there are also special ! and [ bytes that transmit errors and the Arduino's state, respectively
        };

        void connect();
        void (*on_connect)();

        void connected();
        void (*on_connected)();

        void remote_ready();
        void (*on_remote_ready)();

        void ready();
        void (*on_ready)();

        void you_choose(RPA::WEAPON weapon);
        void (*on_you_choose)(RPA::WEAPON weapon);

        void they_choose(RPA::WEAPON weapon);
        void (*on_they_choose)(RPA::WEAPON weapon);

        void you_win();
        void (*on_you_win)();

        void you_lose();
        void (*on_you_lose)();

        void tie();
        void (*on_tie)();

        void disconnected();
        void (*on_disconnected)();

        void (*on_enter_state)(RPA::States::STATE state);
        void (*on_exit_state)(RPA::States::STATE from_state, RPA::States::STATE to_state);

        void check_input_from_serial();

        int win_count;
        int lose_count;

    protected:

        RPA::States::STATE state;

        RPA::WEAPON your_weapon;
        RPA::WEAPON their_weapon;

        void change_state_to(RPA::States::STATE new_state);

        void set_their_weapon(RPA::WEAPON weapon);
        void set_your_weapon(RPA::WEAPON weapon);

        // called when attempting to make an impossible transition
        void invalid_transition_error(String eventName);

        // processes an incoming Protocol byte from Serial
        void in(char c);
        // sends out a Protocol byte followed by \n over Serial
        void out(char c);
        // sends out a string followed by \n over Serial (for longer commands)
        void out(String &str);


};