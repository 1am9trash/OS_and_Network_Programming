#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
using namespace std;

#define pb push_back

struct client_info {
    int socket;
    char ip[24], port[8], name[13];
    client_info() {}
    client_info(int socket, int port, char* ip) {
        this->socket = socket;
        strcpy(this->ip, ip);
        sprintf(this->port, "%d", htons(port));
        strcpy(this->name, "anonymous");
    }
};

bool char_to_int(char *c, int *p) {
    *p = 0;
    for (int i = 0; i < strlen(c); i++) {
        *p *= 10;
        if (c[i] < '0' || c[i] > '9')
            return false;
        *p += c[i] - '0';
    }
    return true;
}

void logout(vector<client_info>& clients, int cur, int word_cnt, char *send_buf) {
    if (word_cnt != 1) {
        strcpy(send_buf, "[Server] USAGE: exit\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }
    cout << "[Server] LOGOUT: " << clients[cur].ip << " " << clients[cur].port << "\n";
    
    clients.erase(clients.begin() + cur);
    strcpy(send_buf, "[Server] ");
    strcat(send_buf, clients[cur].name);
    strcat(send_buf, " is offline.\n");
    for (int i = 0; i < clients.size(); i++)
        send(clients[i].socket, send_buf, strlen(send_buf), 0);
}

void who(vector<client_info>& clients, int cur, int word_cnt, char *send_buf) {
    if (word_cnt != 1) {
        strcpy(send_buf, "[Server] USAGE: who\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }
    cout << "[Server] WHO: " << clients[cur].ip << " " << clients[cur].port << "\n";
    send_buf[0] = '\0';
    for (int i = 0; i < clients.size(); i++) {
        strcat(send_buf, "[Server] ");
        strcat(send_buf, clients[i].name);
        strcat(send_buf, " ");
        strcat(send_buf, clients[i].ip);
        strcat(send_buf, ":");
        strcat(send_buf, clients[i].port);
        if (i == cur)
            strcat(send_buf, " ->me");
        strcat(send_buf, "\n");
    }
    send(clients[cur].socket, send_buf, strlen(send_buf), 0);
}

void yell(vector<client_info>& clients, int cur, int word_cnt, char **word_split, char *send_buf) {
    if (word_cnt < 2) {
        strcpy(send_buf, "[Server] USAGE: yell <MESSAGE>.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }

    strcpy(send_buf, "[Server] ");
    strcat(send_buf, clients[cur].name);
    strcat(send_buf, " yell");
    for (int i = 1; i < word_cnt; i++) {
        strcat(send_buf, " ");
        strcat(send_buf, word_split[i]);
    }
    strcat(send_buf, "\n");
    for (int i = 0; i < clients.size(); i++)
        send(clients[i].socket, send_buf, strlen(send_buf), 0);
}

void tell(vector<client_info>& clients, int cur, int word_cnt, char **word_split, char *send_buf) {
    if (word_cnt < 3) {
        strcpy(send_buf, "[Server] USAGE: tell <USERNAME> <MESSAGE>.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }

    bool is_error = false;
    if (!strcmp(clients[cur].name, "anonymous")) {
        strcpy(send_buf, "[Server] ERROR: You are anonymous.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        is_error = true;
    }
    if (!strcmp(word_split[1], "anonymous")) {
        strcpy(send_buf, "[Server] ERROR: The client to which you sent is anonymous.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        is_error = true;
    }
    if (is_error)
        return;

    int receiver = -1;
    for (int i = 0; i < clients.size(); i++)
        if (!strcmp(clients[i].name, word_split[1])) {
            receiver = i;
            break;
        }
    if (receiver == -1) {
        strcpy(send_buf, "[Server] ERROR: The receiver doesn't exist.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
    } else {
        strcpy(send_buf, "[Server] SUCCESS: Your message has been sent.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        strcpy(send_buf, "[Server] ");
        strcat(send_buf, clients[cur].name);
        strcat(send_buf, " tell you");
        for (int i = 2; i < word_cnt; i++) {
            strcat(send_buf, " ");
            strcat(send_buf, word_split[i]);
        }
        strcat(send_buf, "\n");
        send(clients[receiver].socket, send_buf, strlen(send_buf), 0);
    }
}
 
void name(vector<client_info>& clients, int cur, int word_cnt, char **word_split, char *send_buf) {
    if (word_cnt != 2) {
        strcpy(send_buf, "[Server] USAGE: name <NEW USERNAME>.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }
    cout << "[Server] NAME: " << clients[cur].ip << " " << clients[cur].port << "\n";

    if (!strcmp(word_split[1], "anonymous")) {
        strcpy(send_buf, "[Server] ERROR: Username cannot be anonymous.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }

    bool is_unique = true;
    for (int i = 0; i < clients.size(); i++) {
        if (i == cur)
            continue;
        if (!strcmp(clients[i].name, word_split[1])) {
            is_unique = false;
            break;
        }
    }
    if (!is_unique) {
        strcpy(send_buf, "[Server] ERROR: ");
        strcat(send_buf, word_split[1]);
        strcat(send_buf, " has been used by others.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }

    bool is_all_letter = true;
    for (int i = 0; i < strlen(word_split[1]); i++) 
        if (!((word_split[1][i] >= 'a' && word_split[1][i] <= 'z') || (word_split[1][i] >= 'A' && word_split[1][i] <= 'Z'))) {
            is_all_letter = false;
            break;
        }
    if (!is_all_letter || strlen(word_split[1]) < 2 || strlen(word_split[1]) > 12) {
        strcpy(send_buf, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
        send(clients[cur].socket, send_buf, strlen(send_buf), 0);
        return;
    }

    for (int i = 0; i < clients.size(); i++) {
        strcpy(send_buf, "[Server] ");
        if (i != cur) {
            strcat(send_buf, clients[cur].name);
            strcat(send_buf, " is ");
        }
        else
            strcat(send_buf, "You're ");
        strcat(send_buf, "now known as ");
        strcat(send_buf, word_split[1]);
        strcat(send_buf, ".\n");
        send(clients[i].socket, send_buf, strlen(send_buf), 0);
    }
    strcpy(clients[cur].name, word_split[1]);
}

void error_cmd(vector<client_info>& clients, int cur, char *send_buf) {
    cout << "[Server] ERROR COMMAND: " << clients[cur].ip << " " << clients[cur].port << "\n";
    strcpy(send_buf, "[Server] ERROR: Error command.\n");
    send(clients[cur].socket, send_buf, strlen(send_buf), 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "[Input] USAGE: <program> <port>\n";
        return 1;
    }

    int port;
    if (!char_to_int(argv[1], &port)) {
        cerr << "[Input] ERROR: port must be a positive number.\n";
        return 1;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "[Init] ERROR: socket() error.\n";
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_aton("0.0.0.0", &server_address.sin_addr);

    if (bind(server_socket, (sockaddr *)&server_address, sizeof(server_address)) == -1) {
        cerr << "[Init] ERROR: bind() error.\n";
        return 1;
    }

    if (listen(server_socket, 20) == -1) {
        cerr << "[Init] ERROR: listen() error.\n";
        return 1;
    }

    vector<client_info> clients;
    char recv_buf[4096], send_buf[4096];
    char *line_split[4096], *word_split[4096];

    fd_set rd;
    timeval tv;
    int status, socket_mx;

    while (1) {
        FD_ZERO(&rd);
        FD_SET(server_socket, &rd);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        status = select(server_socket + 1, &rd, NULL, NULL, &tv);

        if (status == 0)
            ;
        else if (status == -1)
            cerr << "[Server] ERROR: select() error.\n";
        else {
            if (FD_ISSET(server_socket, &rd)) {
                sockaddr client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_socket = accept(server_socket, &client_addr, &client_addr_len);

                if (client_socket == -1)
                    cerr << "[Server] ERROR: accept() error.\n";
                else {
                    sockaddr_in* client_addr_in = (sockaddr_in*)&client_addr;
                    clients.pb(client_info(client_socket, client_addr_in->sin_port, inet_ntoa(client_addr_in->sin_addr)));
                    cout << "[Server] LOGIN: " << clients[clients.size() - 1].ip << " " << clients[clients.size() - 1].port << "\n";

                    strcpy(send_buf, "[Server] Hello, anonymous! From: ");
                    strcat(send_buf, clients[clients.size() - 1].ip);
                    strcat(send_buf, ":");
                    strcat(send_buf, clients[clients.size() - 1].port);
                    strcat(send_buf, "\n");
                    send(client_socket, send_buf, strlen(send_buf), 0);

                    strcpy(send_buf, "[Server] Someone is coming!\n");
                    for (int i = 0; i < clients.size() - 1; i++)
                        send(clients[i].socket, send_buf, strlen(send_buf), 0);
                }
            }
        }

        FD_ZERO(&rd);
        socket_mx = 0;
        for (int i = 0; i < clients.size(); i++) {
            socket_mx = max(socket_mx, clients[i].socket);
            FD_SET(clients[i].socket, &rd);
        }
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        status = select(socket_mx + 1, &rd, NULL, NULL, &tv);

        if (status == 0)
            ;
        else if (status == -1)
            cerr << "[Client Handler] ERROR: select() error.\n";
        else {
            for (int i = 0; i < clients.size(); i++) {
                if (!FD_ISSET(clients[i].socket, &rd))
                    continue;
                memset(recv_buf, 0, sizeof(recv_buf));

                int nbyte = recv(clients[i].socket, recv_buf, sizeof(recv_buf) - 1, 0);
                if (nbyte <= 0) {
                    logout(clients, i, 1, send_buf);
                    continue;
                }

                recv_buf[nbyte - 1] = '\0';
                int line_cnt = 0;
                char *save_p = NULL;
                char *p = strtok_r(recv_buf, "\n", &save_p);
                while (p != NULL) {
                    if (strlen(p) == 0)
                        continue;
                    line_split[line_cnt++] = p;
                    p = strtok_r(NULL, "\n", &save_p);
                }

                for (int j = 0; j < line_cnt; j++) {
                    int word_cnt = 0;
                    save_p = NULL;
                    p = strtok_r(line_split[j], " ", &save_p);
                    while (p != NULL) {
                        if (strlen(p) == 0)
                            continue;
                        word_split[word_cnt++] = p;
                        p = strtok_r(NULL, " ", &save_p);
                    }

                    if (word_cnt < 1)
                        continue;

                    if (!strcmp(word_split[0], "who"))
                        who(clients, i, word_cnt, send_buf);
                    else if (!strcmp(word_split[0], "name"))
                        name(clients, i, word_cnt, word_split, send_buf);
                    else if (!strcmp(word_split[0], "tell"))
                        tell(clients, i, word_cnt, word_split, send_buf);
                    else if (!strcmp(word_split[0], "yell"))
                        yell(clients, i, word_cnt, word_split, send_buf);
                    else if (!strcmp(word_split[0], "exit"))
                        logout(clients, i, word_cnt, send_buf);
                    else
                        error_cmd(clients, i, send_buf);
                }
            }
        }
    }
    return 0;
}
