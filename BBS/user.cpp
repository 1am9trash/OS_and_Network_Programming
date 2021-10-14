#include "user.h"

USER::USER() {}

USER::USER(char *username, char *password) {
    this->username = new char[strlen(username) + 1];
    strcpy(this->username, username);
    this->password = new char[strlen(password) + 1];
    strcpy(this->password, password);
}

void USER::send_message(char *sender, char *message) {
    for (int i = 0; i < this->message_boxes.size(); i++)
        if (!strcmp(sender, this->message_boxes[i].sender)) {
            this->message_boxes[i].add_message(message);
            return;
        }
    MESSAGEBOX new_message_box(sender);
    new_message_box.add_message(message);
    this->message_boxes.pb(new_message_box);
}