#include "User.h"

User::User() {
    nick = "nircer";
}

bool User::setNick(string newNick) {
    // TODO: check validity (forbidden chars, length)
    nick = newNick;

    return true;
}

string User::getNick() {
    return nick;
}
