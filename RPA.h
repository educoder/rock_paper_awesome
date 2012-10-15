#include <stdio.h>
#include "Arduino.h"

class RPA {
    public:
        RPA();

        enum WEAPON { ROCK, PAPER, SCISSORS };

        struct States {
            enum STATE {
                OFFLINE = 0,
                WAITING_FOR_XMPP_CONNECTION = 1,
                WAITING_FOR_EITHER_READY = 2,
                WAITING_FOR_THEIR_READY = 3,
                WAITING_FOR_YOUR_READY = 4,
                WAITING_FOR_EITHER_CHOICE = 5,
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

                static const char INVALID_TRANSITION = '!'; // sent when an invalid transition occurs
        };

        void connect();
        void (*on_connect)();

        void online();
        void (*on_online)();

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

        void offline();
        void (*on_offline)();

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
        void invalid_transition_error(const char* eventName);

        // processes an incoming Protocol byte from Serial
        void in(char c);
        // sends out a Protocol byte over Seiral
        void out(char c);


};