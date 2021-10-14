#ifndef USER_H
#define USER_H

#include "messagebox.h"
#include <cstring>
#include <vector>
using namespace std;

class USER {
public:
    char *username;
    char *password;
    vector<MESSAGEBOX> message_boxes;

    USER();
    USER(char *username, char *password);
    void send_message(char *sender, char *message);
};

#endif