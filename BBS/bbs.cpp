#include "bbs.h"

ARGS::ARGS() {}

ARGS::ARGS(void *p, int client_socket) {
    this->p = p;
    this->client_socket = client_socket;
}

BBS::BBS() {
    this->init();
}

BBS::BBS(int max_user, int port, char *IP) {
    this->init();
    this->max_user = max_user;
    this->port = port;
    this->IP = new char[strlen(IP) + 1];
    strcpy(this->IP, IP);
}

void BBS::init() {
    this->cur_user = 0;
}

bool BBS::register_user(char *username, char *password) {
    pthread_rwlock_wrlock(&users_rwlock);
    for (int i = 0; i < this->users.size(); i++)
        if (!strcmp(username, this->users[i]->username)) {
            pthread_rwlock_unlock(&users_rwlock);
            return false;
        }
    this->users.pb(new USER(username, password));
    pthread_rwlock_unlock(&users_rwlock);
    return true;
}

USER *BBS::login(char *username, char *password) {
    pthread_rwlock_rdlock(&users_rwlock);
    for (int i = 0; i < this->users.size(); i++)
        if (!strcmp(username, this->users[i]->username) && !strcmp(password, this->users[i]->password)) {
            pthread_rwlock_unlock(&users_rwlock);
            return this->users[i];
        }
    pthread_rwlock_unlock(&users_rwlock);
    return NULL;
}

void BBS::connect_handler(int client_socket) {
    char send_buf[4096];
    strcpy(send_buf, "********************************\n");
    send(client_socket, send_buf, strlen(send_buf), 0);
    strcpy(send_buf, "** Welcome to the BBS server. **\n");
    send(client_socket, send_buf, strlen(send_buf), 0);
    strcpy(send_buf, "********************************\n");
    send(client_socket, send_buf, strlen(send_buf), 0);

    char buf[4096];
    char *split[4096];
    USER *user = NULL;

    while (1) {
        strcpy(send_buf, "% ");
        send(client_socket, send_buf, strlen(send_buf), 0);
        memset(buf, 0, sizeof(buf));
        if (recv(client_socket, &buf, sizeof(buf), 0) <= 0)
            break;

        buf[strlen(buf) - 1] = '\0';
        int cnt = 0;
        char *p = strtok(buf, " ");
        while (p != NULL) {
            if (strlen(p) == 0)
                continue;
            split[cnt++] = p;
            p = strtok(NULL, " ");
        }

        if (!strcmp(split[0], "register")) {
            if (cnt != 3) {
                strcpy(send_buf, "Usage: register <username> <password>\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else if (this->register_user(split[1], split[2])) {
                strcpy(send_buf, "Register successfully.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else {
                strcpy(send_buf, "Username is already used.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            }
        } else if (!strcmp(split[0], "login")) {
            if (cnt != 3) {
                strcpy(send_buf, "Usage: login <username> <password>\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else if (user != NULL) {
                strcpy(send_buf, "Please logout first.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else {
                user = this->login(split[1], split[2]);
                if (user != NULL) {
                    strcpy(send_buf, "Welcome, ");
                    pthread_rwlock_rdlock(&users_rwlock);
                    strcat(send_buf, user->username);
                    pthread_rwlock_unlock(&users_rwlock);
                    strcat(send_buf, ".\n");
                    send(client_socket, send_buf, strlen(send_buf), 0);
                } else {
                    strcpy(send_buf, "Login failed.\n");
                    send(client_socket, send_buf, strlen(send_buf), 0);
                }
            }
        } else if (!strcmp(split[0], "whoami")) {
            if (user != NULL) {
                pthread_rwlock_rdlock(&users_rwlock);
                strcpy(send_buf, user->username);
                pthread_rwlock_unlock(&users_rwlock);
                strcat(send_buf, "\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else {
                strcpy(send_buf, "Please login first.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            }
        } else if (!strcmp(split[0], "logout")) {
            if (user != NULL) {
                strcpy(send_buf, "Bye, ");
                pthread_rwlock_rdlock(&users_rwlock);
                strcat(send_buf, user->username);
                user = NULL;
                pthread_rwlock_unlock(&users_rwlock);
                strcat(send_buf, ".\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else {
                strcpy(send_buf, "Please login first.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            }
        } else if (!strcmp(split[0], "list-user")) {
            pthread_rwlock_rdlock(&users_rwlock);
            strcpy(send_buf, "");
            for (int i = 0; i < this->users.size(); i++) {
                strcat(send_buf, this->users[i]->username);
                strcat(send_buf, "\n");
            }
            pthread_rwlock_unlock(&users_rwlock);
            send(client_socket, send_buf, strlen(send_buf), 0);
        } else if (!strcmp(split[0], "send")) {
            if (cnt < 3) {
                strcpy(send_buf, "Usage: send <username> <message>\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else if (user != NULL) {
                strcpy(send_buf, split[2]);
                for (int i = 2; i < cnt; i++) {
                    strcat(send_buf, " ");
                    strcat(send_buf, split[i]);
                }
                pthread_rwlock_wrlock(&users_rwlock);
                bool find = false;
                for (int i = 0; i < this->users.size(); i++)
                    if (!strcmp(this->users[i]->username, split[1])) {
                        find = true;
                        this->users[i]->send_message(user->username, send_buf);
                        break;
                    }
                if (!find) {
                    strcpy(send_buf, "User not existed.\n");
                    send(client_socket, send_buf, strlen(send_buf), 0);
                }
                pthread_rwlock_unlock(&users_rwlock);
            } else {
                strcpy(send_buf, "Please login first.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            }
        } else if (!strcmp(split[0], "list-msg")) {
            if (user != NULL) {
                send_buf[0] = '\0';
                pthread_rwlock_rdlock(&users_rwlock);
                for (int i = 0; i < user->message_boxes.size(); i++) {
                    char cnt_to_char[10];
                    sprintf(cnt_to_char, "%lu", user->message_boxes[i].messages.size());
                    strcat(send_buf, cnt_to_char);
                    strcat(send_buf, " message from ");
                    strcat(send_buf, user->message_boxes[i].sender);
                    strcat(send_buf, ".\n");
                }
                pthread_rwlock_unlock(&users_rwlock);
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else {
                strcpy(send_buf, "Please login first.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            }
        } else if (!strcmp(split[0], "receive")) {
            if (cnt != 2) {
                strcpy(send_buf, "Usage: receive <username>\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            } else if (user != NULL) {
                pthread_rwlock_wrlock(&users_rwlock);
                bool find = false;
                for (int i = 0; i < user->message_boxes.size(); i++)
                    if (!strcmp(user->message_boxes[i].sender, split[1])) {
                        find = true;
                        send_buf[0] = '\0';
                        for (int j = 0; j < user->message_boxes[i].messages.size(); j++) {
                            strcat(send_buf, user->message_boxes[i].messages[j]);
                            strcat(send_buf, "\n");
                        }
                        send(client_socket, send_buf, strlen(send_buf), 0);
                        break;
                    }
                if (!find) {
                    strcpy(send_buf, "User didn't send message to you.\n");
                    send(client_socket, send_buf, strlen(send_buf), 0);
                }
                pthread_rwlock_unlock(&users_rwlock);
            } else {
                strcpy(send_buf, "Please login first.\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            }
        } else if (!strcmp(split[0], "exit")) {
            if (user != NULL) {
                strcpy(send_buf, "Bye, ");
                pthread_rwlock_rdlock(&users_rwlock);
                strcat(send_buf, user->username);
                pthread_rwlock_unlock(&users_rwlock);
                strcat(send_buf, ".\n");
                send(client_socket, send_buf, strlen(send_buf), 0);
            }
            break;
        } else if (cnt != 0) {
            strcpy(send_buf, "Don't have this commend.\n");
            send(client_socket, send_buf, strlen(send_buf), 0);
        }
    }
    pthread_mutex_lock(&cnt_mutex);
    this->cur_user--;
    pthread_mutex_unlock(&cnt_mutex);
    close(client_socket);
}

void *BBS::static_connect_handler(void *args) {
    ARGS *thread_args = (ARGS *)args;
    static_cast<BBS *>(thread_args->p)->connect_handler(thread_args->client_socket);
    return NULL;
}

void BBS::run() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "socket() error.\n";
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->port);
    inet_aton(this->IP, &server_address.sin_addr);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        cerr << "bind() error.\n";
        return;
    }

    if (listen(server_socket, this->max_user) == -1) {
        cerr << "listen() error.\n";
        return;
    }

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            cerr << "accept() error.\n";
            return;
        }

        if (this->cur_user >= max_user) {
            cout << "Reach user limit.\n";
            close(client_socket);
        } else {
            this->cur_user++;
            pthread_t tid;
            ARGS args(this, client_socket);
            pthread_create(&tid, NULL, this->static_connect_handler, &args);
        }
    }
    close(server_socket);
}