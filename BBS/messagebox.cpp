#include "messagebox.h"

MESSAGEBOX::MESSAGEBOX() {}

MESSAGEBOX::MESSAGEBOX(char *sender) {
    this->sender = new char[strlen(sender) + 1];
    strcpy(this->sender, sender);
}

void MESSAGEBOX::add_message(char *message) {
    char *buf = new char[strlen(message) + 1];
    strcpy(buf, message);
    this->messages.pb(buf);
}