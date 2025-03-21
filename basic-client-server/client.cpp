#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <ctime>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

using namespace std;

#define PORT 8081
#define SERVER_IP "127.0.0.3"
time_t start, finish;

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

void receive_messages(int conn_socket) {
    while (true) {
        char buffer[256] = {};
        ssize_t n = read(conn_socket, buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            printf("[-] Server disconnected. Closing chat...\n");
            close(conn_socket);
            exit(0);
        }

        printf("\nServer: %s\nYou: ", buffer);
        fflush(stdout);
    }
}

void send_messages(int conn_socket) {
    while (true) {
        string message;
        cout << "You: ";
        getline(cin, message);

        ssize_t n = write(conn_socket, message.c_str(), message.size());

        if(n < 0) {
            printf("[-] Server disconnected. Closing chat...\n");
            close(conn_socket);
            exit(0);
        }
    }
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    inet_pton(AF_INET, SERVER_IP, &(addr.sin_addr.s_addr));

    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    printf("[+] Connected to server! Start chatting...\n");

    thread reciever(receive_messages, fd);
    thread sender(send_messages, fd);

    close(fd);
    return 0;
}