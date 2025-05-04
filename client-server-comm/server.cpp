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

int main(int argc, char* argv[]){
    int initial_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (initial_socket < 0) {
        fprintf(stderr, "socket() failed\n");
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &(addr.sin_addr.s_addr));
    socklen_t addr_len = sizeof(addr);

    int rv = bind(initial_socket, (const struct sockaddr *)&addr, addr_len);
    if (rv) {
        fprintf(stderr, "bind() failed\n");
        exit(1);
    } else {
        printf("[+] binded to port:%d\n", PORT);
    }

    rv = listen(initial_socket, SOMAXCONN);
    if (rv) {
        fprintf(stderr, "listen() failed\n");
        exit(1);
    } else {
        printf("[+] server is listening...\n");
    }

    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);

    cout << "waiting on accept syscall";
    int client_sock = accept(initial_socket, (struct sockaddr*)&client_addr, &socklen);
    if (client_sock < 0) {
        fprintf(stderr, "[-] Connection for client with socket %d failed.\n", client_sock);
    } else {
        printf("[+] 1 new connection added: %d\n", client_sock);
    }
    cout << "after accept";

    while(true) {
        char buffer[256] = {0};
        ssize_t n = read(client_sock, buffer, sizeof(buffer) - 1);
        if (n < 0) {
            perror("read() failed");
        } else if (n == 0) {
            cout << "Client disconnected\n";
            break;
        } 

        printf("Client: %s\nYou: ", buffer);
        fflush(stdout);
        
        string msg;
        getline(cin,msg);
        if (msg.empty()) continue; // Avoid sending empty messages
    
        write(client_sock, msg.c_str(), msg.size());
        
        if (msg == "exit") break;
    }
    
    printf("[-] connection terminated with client %d\n", client_sock);
    close(client_sock);
    close(initial_socket);
    printf("[-] server shutting down...\n");
    return 0;
}