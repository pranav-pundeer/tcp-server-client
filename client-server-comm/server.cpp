#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

using namespace std;

#define SERVER_IP "127.0.0.3"

int main(int argc, char* argv[]){
    if(argc < 2) {
        fprintf(stderr, "Port number not provided. Provide a port number to bind to.\n");
        exit(1);
    }

    int port = atoi(argv[1]);
    int initial_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (initial_socket < 0) {
        fprintf(stderr, "socket() failed\n");
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, SERVER_IP, &(addr.sin_addr.s_addr));
    socklen_t addr_len = sizeof(addr);

    int rv = bind(initial_socket, (const struct sockaddr *)&addr, addr_len);
    if (rv) {
        fprintf(stderr, "bind() failed\n");
        exit(1);
    } else {
        printf("[+] binded to port:%d\n", port);
    }

    rv = listen(initial_socket, SOMAXCONN);
    if (rv) {
        fprintf(stderr, "listen() failed\n");
        exit(1);
    } else {
        printf("[+] server is listening...\n");
    }

    while(true) {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);

        int client_sock = accept(initial_socket, (struct sockaddr*)&client_addr, &socklen);
        if (client_sock < 0) {
            fprintf(stderr, "[-] Connection for client with socket %d failed.\n", client_sock);
        } else {
            printf("[+] 1 new connection added: %d\n", client_sock);
        }

        while(true) {
            char buffer[256];
            int n = read(client_sock, buffer, sizeof(buffer) - 1);
            if(n<0) {
                fprintf(stderr, "read() failed\n");
            }
            
            printf("Client: %s\nYou: ", buffer);
            fflush(stdout);
            
            string msg;
            getline(cin, msg);
            memcpy(buffer, msg.c_str(), msg.size());
            buffer[msg.length()] = '\0';

            n = write(client_sock, buffer, strlen(buffer));
            if(n<0) {
                fprintf(stderr, "write() failed\n");
            }

            int close_conn = strcmp("exit", buffer);

            if(close_conn == 0) break;
        }
        printf("[-] connection terminated with client %d\n", client_sock);
        close(client_sock);
    }
    printf("[-] server shutting down...\n");
    close(initial_socket);
    return 0;
}