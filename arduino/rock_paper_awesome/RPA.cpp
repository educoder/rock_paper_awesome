#include "RPA.h" 


RPA::RPA() {
    state = RPA::States::OFFLINE;
    win_count = 0;
    lose_count = 0;
}

void RPA::connect() {
    switch (state) {
        case RPA::States::OFFLINE:
            out(RPA::Protocol::ARDUINO_ON);
            if (*on_connect) on_connect();
            change_state_to(RPA::States::CONNECTING);
            break;
        default:
            invalid_transition_error("connect");
    }
}

void RPA::connected() {
    switch (state) {
        case RPA::States::CONNECTING:
            change_state_to(RPA::States::ONLINE);
            if (*on_connected) on_connected();
            break;
        default:
            invalid_transition_error("connected");
    }
}

void RPA::remote_ready() {
    switch (state) {
        case RPA::States::ONLINE:
            change_state_to(RPA::States::WAITING_FOR_YOUR_READY);
            if (*on_remote_ready) on_remote_ready();
            break;
        case RPA::States::WAITING_FOR_THEIR_READY:
            change_state_to(RPA::States::READY_TO_PLAY);
            if (*on_remote_ready) on_remote_ready();
            break;
        default:
            invalid_transition_error("remote_ready");
    }
}

void RPA::ready() {
    switch (state) {
        case RPA::States::ONLINE:
            out(RPA::Protocol::READY_FOR_NEW_GAME);
            change_state_to(RPA::States::WAITING_FOR_THEIR_READY);
            if (*on_ready) on_ready();
            break;
        case RPA::States::WAITING_FOR_YOUR_READY:
            out(RPA::Protocol::READY_FOR_NEW_GAME);
            change_state_to(RPA::States::READY_TO_PLAY);
            if (*on_ready) on_ready();
            break;
        default:
            invalid_transition_error("ready");
    }
}

void RPA::you_choose(RPA::WEAPON weapon) {
    switch (state) {
        case RPA::States::READY_TO_PLAY:
            set_your_weapon(weapon);
            if (*on_you_choose) on_you_choose(weapon);
            change_state_to(RPA::States::WAITING_FOR_THEIR_CHOICE);
            break;
        case RPA::States::WAITING_FOR_YOUR_CHOICE:
            set_your_weapon(weapon);
            if (*on_you_choose) on_you_choose(weapon);
            change_state_to(RPA::States::WAITING_FOR_RESULT);
            break;
        default:
            String eventName = "you_chose(";
            eventName.concat((char) weapon); // hacky concat int as char
            eventName.concat(")");
            invalid_transition_error(eventName);
    }
}

void RPA::they_choose(RPA::WEAPON weapon) {
    switch (state) {
        case RPA::States::READY_TO_PLAY:
            if (*on_they_choose) on_they_choose(weapon);
            change_state_to(RPA::States::WAITING_FOR_YOUR_CHOICE);
            break;
        case RPA::States::WAITING_FOR_THEIR_CHOICE:
            if (*on_they_choose) on_they_choose(weapon);
            change_state_to(RPA::States::WAITING_FOR_RESULT);
            break;
        default:
            invalid_transition_error("they_chose");
    }
}

void RPA::you_win() {
    switch (state) {
        case RPA::States::WAITING_FOR_RESULT:
            win_count++;
            if (*on_you_win) on_you_win();
            change_state_to(RPA::States::ONLINE);
            break;
        default:
            invalid_transition_error("you_win");
    }
}

void RPA::you_lose() {
    switch (state) {
        case RPA::States::WAITING_FOR_RESULT:
            lose_count++;
            if (*on_you_lose) on_you_lose();
            change_state_to(RPA::States::ONLINE);
            break;
        default:
            invalid_transition_error("you_lose");
    }
}

void RPA::tie() {
    switch (state) {
        case RPA::States::WAITING_FOR_RESULT:
            if (*on_tie) on_tie();
            change_state_to(RPA::States::ONLINE);
            break;
        default:
            invalid_transition_error("tie");
    }
}

void RPA::disconnected() {
    // special case - this is avalid transition from all states:
    // we just go straight to offline state if this happens
    // and immediatley try to reconnect
    change_state_to(RPA::States::OFFLINE);
    if (*on_disconnected) on_disconnected();
    connect();
}


void RPA::check_input_from_serial() {
    if (Serial.available() > 0) {
        in(Serial.read());
    }
}

void RPA::change_state_to(RPA::States::STATE new_state) {
    if (*on_exit_state) on_exit_state(state, new_state);
    state = new_state;

    String state_update = "[";
    state_update.concat(new_state);
    out(state_update); // FIXME: hacky, and can only handle up to 10 states (0 - 9)

    if (*on_enter_state) on_enter_state(state);
}

void RPA::set_their_weapon(RPA::WEAPON weapon) {
    their_weapon = weapon;
}

void RPA::set_your_weapon(RPA::WEAPON weapon) {
    switch (weapon) {
        case RPA::ROCK:
            out(RPA::Protocol::YOU_CHOSE_ROCK);
            break;
        case RPA::PAPER:
            out(RPA::Protocol::YOU_CHOSE_PAPER);
            break;
        case RPA::SCISSORS:
            out(RPA::Protocol::YOU_CHOSE_SCISSORS);
            break;
    }

    //your_weapon = weapon;
}

void RPA::invalid_transition_error(String eventName) {
    String err = "!" + eventName;
    out(err);
}

// processes an incoming Protocol byte from Serial
void RPA::in(char c) {
    switch (c) {
        case RPA::Protocol::ONLINE:
            connected();
            break;
        case RPA::Protocol::REMOTE_READY:
            remote_ready();
            break;
        case RPA::Protocol::THEY_CHOSE_ROCK:
        case RPA::Protocol::THEY_CHOSE_PAPER:
        case RPA::Protocol::THEY_CHOSE_SCISSORS:
            they_choose((RPA::WEAPON) c);
            break;
        // case RPA::Protocol::WAITING_FOR_YOU:
        //     waitingForYou();
        //     break;
        // case RPA::Protocol::WAITING_FOR_THEM:
        //     waitingForThem();
        //     break;
        case RPA::Protocol::YOU_WON:
            you_win();
            break;
        case RPA::Protocol::YOU_LOST:
            you_lose();
            break;
        case RPA::Protocol::TIE:
            tie();
            break;
        case RPA::Protocol::OFFLINE:
            disconnected();
            break;
      }
}

// sends out a Protocol byte followed by \n over Serial
void RPA::out(char c) {
    Serial.println(c);
}

// sends out a string followed by \n over Serial (for longer commands)
void RPA::out(String &str) {
    Serial.println(str);
}