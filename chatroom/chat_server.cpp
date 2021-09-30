#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>

#include <iostream>
#include <vector>

using namespace std;


#define IP "0.0.0.0"
#define PORT 9001
#define BACKLOG 5
#define MX 5

#define pb push_back


vector<int> fds;

void *pthread_service(void *arg) {
    int client_socket = *(int*)arg;

    char server_message[256] = "welcome to server\n\n";
    send(client_socket, server_message, strlen(server_message), 0);
    
    char buf[1024];

    while (1) {
        memset(buf, 0, sizeof(buf));
        if (recv(client_socket, &buf, sizeof(buf), 0) <= 0)
            break;
        if (!strcmp(buf, "exit")) {
            for (int i = 0; i < fds.size(); i++)
                if (fds[i] == client_socket) {
                    fds.erase(fds.begin() + i);
                    break;
                }
            send(client_socket, buf, strlen(buf), 0);
            break;
        }
        for (int i = 0; i < fds.size(); i++) {
            if (fds[i] == client_socket)
                continue;
            send(fds[i], buf, strlen(buf), 0);
        }
    }

    cout << "client socket " << client_socket << " closed\n";
    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "socket() error\n";
        return 1;
    }
        
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_aton(IP, &server_address.sin_addr);
    // server_address.sin_addr.s_addr = inet_addr(IP);

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        cerr << "bind() error\n";
        return 1;
    }

    if (listen(server_socket, BACKLOG) == -1) {
        cerr << "listen() error\n";
        return 1;
    }

    int cnt = 0;
    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            cerr << "accept() error\n";
            return 1;
        }
        if (fds.size() >= MX) {
            cout << "no more clients is allowed\n";
            continue;
        }

        fds.pb(client_socket);

        pthread_t tid;
        pthread_create(&tid, NULL, pthread_service, &client_socket);
    }

    close(server_socket);
    
    return 0;
}
