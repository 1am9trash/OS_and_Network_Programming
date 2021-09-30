#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

using namespace std;


#define IP "192.168.0.113"
#define PORT 9001

#define WHITE "\033[37m"
#define YELLOW "\033[33m"   
#define RED "\033[31m"


void *pthread_service(void *arg) {
    int client_socket = *(int*)arg;
    char buf[1024];
    while (1) {
        memset(buf, 0, sizeof(buf));
        if (recv(client_socket, buf, sizeof(buf), 0) == -1) {
            cerr << RED << "recv() error\n";
            break;
        }
        if (!strcmp(buf, "exit"))
            break;
        cout << YELLOW << buf;
        cout << WHITE;
        cout.flush();
    }

    cout << "close client socket\n";
    cout << "close thread\n";
    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_aton(IP, &server_address.sin_addr);
    // server_address.sin_addr.s_addr = inet_addr(IP);

    int connect_status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connect_status == -1) {
        cerr << RED << "connect() error\n";
        return 1;
    }

    char name[1024];
    memset(name, 0, sizeof(name));
    cout << YELLOW << "enter your username\n";
    cout << WHITE;
    cin >> name;
    cin.ignore();

    pthread_t tid;
    pthread_create(&tid, NULL, pthread_service, &client_socket);

    send(client_socket, "system: ", 8, 0);
    send(client_socket, name, strlen(name), 0);
    send(client_socket, " join the server\n", 18, 0);

    char buf[1024];
    while (1) {
        memset(buf, 0, sizeof(buf));
        cin.getline(buf, sizeof(buf));

        if (!strcmp(buf, "exit")) {
            send(client_socket, buf, strlen(buf), 0);
            break;
        }

        if (send(client_socket, name, strlen(name), 0) == -1) {
            cerr << RED << "send() error\n";
            break;
        }
        if (send(client_socket, ": ", 2, 0) == -1) {
            cerr << RED << "send() error\n";
            break;
        }
        if (send(client_socket, buf, strlen(buf), 0) == -1) {
            cerr << RED << "send() error\n";
            break;
        }
        if (send(client_socket, "\n", 1, 0) == -1) {
            cerr << RED << "send() error\n";
            break;
        }
    }

    return 0;
}
