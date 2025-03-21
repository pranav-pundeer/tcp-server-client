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

#define PORT 8081
#define SERVER_IP "127.0.0.3"
#define SERVER_UP_TIME 300

static int initial_socket = -1;

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void get_ip_port(int s) {
    socklen_t len;
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN]; 
    int port;

    len = sizeof(addr);
    int res = getpeername(s, (struct sockaddr*)&addr, &len);
    if (res < 0) {
        perror("getpeername failed");
        return;
    }

    cout << "addr.ss_family: " << addr.ss_family << endl;

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
    } else {
        cout << "Unknown address family" << endl;
        return;
    }

    cout << "Peer IP address: " << ipstr << endl;
    cout << "Peer port: " << port << endl;
}

static void handle_client(int conn_socket) {
    time_t start, curr_time;
    time(&start);

    while(true) {
        time(&curr_time);
        if(difftime(curr_time, start) >= SERVER_UP_TIME) {
            printf("[-] closing connection...\n");
            break;
        }

        char rbuf[64] = {};
        ssize_t n = read(conn_socket, rbuf, sizeof(rbuf) - 1);
        if (n < 0) {
            fprintf(stderr, "read failed\n");
            break;
        }
        fprintf(stderr, "client says: %s\n", rbuf);

        char wbuf[] = "hi client...";
        write(conn_socket, wbuf, strlen(wbuf));
    }
    close(conn_socket);
}


void receive_messages(int conn_socket) {
    while (true) {
        char buffer[256] = {};
        ssize_t n = read(conn_socket, buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            printf("[-] Client disconnected. Closing chat...\n");
            close(conn_socket);
            exit(0);
        }

        printf("\nClient: %s\nYou: ", buffer);
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
            printf("[-] Client disconnected. Closing chat...\n");
            close(conn_socket);
            exit(0);
        }
    }
}

int main() {
    initial_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (initial_socket < 0) 
        die("socket()");
    
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    
    inet_pton(AF_INET, SERVER_IP, &(addr.sin_addr.s_addr));
    
    int rv = bind(initial_socket, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) 
        die("bind()");
    else 
        printf("[+] binded to %s:%d\n", SERVER_IP, PORT);
    
    rv = listen(initial_socket, SOMAXCONN); //this is where the TCP handshake occurs.
    if (rv) 
        die("listen()");
    else
        printf("[+] listening...\n");

    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);

    //accept connection.
    int conn_socket = accept(initial_socket, (struct sockaddr*)&client_addr, &socklen);

    if (conn_socket < 0) {
        fprintf(stderr, "[-]Connection for client with socket %d failed.\n", conn_socket);
    } else {
        printf("[+] 1 new connection added: %d\n", conn_socket);
    }

    // get_ip_port(conn_socket);
    // handle_client(conn_socket);

    thread receiver(receive_messages, conn_socket);
    thread sender(send_messages, conn_socket);

    receiver.join();
    sender.join();

    shutdown(initial_socket, SHUT_RDWR);
    close(initial_socket);
    printf("[-] server closed successfully.\n");

    return 0;
}