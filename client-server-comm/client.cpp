#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define PORT 8100

int main(int argc, char* argv[]) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0) {
        fprintf(stderr, "socket() failed\n");
        exit(1);
    }

    struct sockaddr_in client_addr = {};
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    socklen_t addr_len = sizeof(client_addr);
    
    inet_pton(AF_INET, SERVER_IP, &(client_addr.sin_addr.s_addr));

    int rv = connect(client_socket, (const struct sockaddr *)&client_addr, addr_len);
    if(rv) {
        fprintf(stderr, "connect() failed\n");
        exit(1);
    } else {
        printf("[+] Connected to server\n");
    }

    while(true) {
        string msg;
        getline(cin, msg);
        if (msg.empty()) continue;
    
        ssize_t bytes_written = write(client_socket, msg.c_str(), msg.size());

        if (bytes_written < 0)
            perror("write() failed"); 
    
        if (msg == "exit") break;

        char buffer[256] = {0}; 
        ssize_t n = read(client_socket, buffer, sizeof(buffer) - 1);
        
        if(n <= 0) {
            if (n == 0) {
                printf("Server closed the connection.\n");
            } else {
                perror("read() failed");
            }
            break;
        }
    
        buffer[n] = '\0';
        printf("Server: %s\nYou: ", buffer);
        fflush(stdout);
    
    }
    printf("[-] Closing connection from client side.\n");
    close(client_socket);
    return 0;
}