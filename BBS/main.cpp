#include "bbs.h"
#include "utils.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4)
        cout << "Usage: <program> <IP> <port> <max user>\n";
    else {
        int port, max_user;
        if (!char_to_int(argv[2], &port))
            cout << "Wrong port input.";
        else if (!char_to_int(argv[3], &max_user))
            cout << "Wrong max user limit input.";
        else {
            BBS server(max_user, port, argv[1]);
            server.run();
        }
    }

    return 0;
}
