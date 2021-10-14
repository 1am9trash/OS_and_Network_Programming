#ifndef BBS_H
#define BBS_H

#include "user.h"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
using namespace std;

struct ARGS {
    void *p;
    int client_socket;
    ARGS();
    ARGS(void *p, int client_socket);
};

class BBS {
public:
    int max_user;
    int cur_user;
    int port;
    char *IP;
    pthread_rwlock_t users_rwlock;
    pthread_mutex_t cnt_mutex;
    vector<USER *> users;

    BBS();
    BBS(int max_user, int port, char *IP);
    void init();
    bool register_user(char *username, char *password);
    USER *login(char *username, char *password);
    void connect_handler(int client_socket);
    static void *static_connect_handler(void *args);
    void run();
};

#endif