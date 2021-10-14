#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <cstring>
#include <vector>
using namespace std;

#define pb push_back

class MESSAGEBOX {
public:
    char *sender;
    vector<char *> messages;

    MESSAGEBOX();
    MESSAGEBOX(char *sender);
    void add_message(char *message);
};

#endif