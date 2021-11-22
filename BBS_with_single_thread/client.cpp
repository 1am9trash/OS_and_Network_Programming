#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
using namespace std;

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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "[Input] USAGE: <program> <IP> <port>\n";
        return 1;
    }

    int port;
    if (!char_to_int(argv[2], &port)) {
        cerr << "[Input] ERROR: port must be a positive number.\n";
        return 1;
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        cerr << "[Init] ERROR: socket() error.\n";
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_aton(argv[1], &server_address.sin_addr);

    if (connect(client_socket, (sockaddr *)&server_address, sizeof(server_address)) == -1){
        cerr << "[Init] ERROR: connect() error.\n";
        return 1;
    }

    char recv_buf[4096], send_buf[4096], check[5];

    fd_set rd;
    timeval tv;
    int status;

    while (1) {
        FD_ZERO(&rd);
        FD_SET(0, &rd);
        FD_SET(client_socket, &rd);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        status = select(client_socket + 1, &rd, NULL, NULL, &tv);

        if (status == 0)
            ;
        else if (status == -1)
            ;
        else {
            if (FD_ISSET(0, &rd)) {
                memset(send_buf, 0, sizeof(send_buf));
                fgets(send_buf, sizeof(send_buf) - 1, stdin);
                send(client_socket, send_buf, strlen(send_buf), 0);

                memcpy(check, send_buf, 4);
                if (!strcmp(check, "exit")) {
                    close(client_socket);
                    return 0;
                }
            }
            if (FD_ISSET(client_socket, &rd)) {
                memset(recv_buf, 0, sizeof(recv_buf));
                if (recv(client_socket, recv_buf, sizeof(recv_buf) - 1, 0) <= 0)
                    continue;
                cout << recv_buf;
            }
        }
    }


    return 0;
}
