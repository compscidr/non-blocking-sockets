//
// Created by jason on 5/27/21.
//

#include "tcp_blocking_server.h"
#include "tcp_client.h"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void tcp_blocking_server::errExit(std::string s) {
    std::cout << s << std::endl;
    exit(1);
}

tcp_blocking_server::tcp_blocking_server() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( 8888);
    if (bind(server_socket, (struct sockaddr *)&address,sizeof(address)) < 0) {
        errExit("bind");
    }
}
void tcp_blocking_server::listen() {
    running = true;
    std::cout << "Listening" << std::endl;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    if(::listen(server_socket, 3) < 0) {
        errExit("listen");
    }
    listen_thread = std::thread([&]() {
        while (running) {
            std::cout << "Waiting for connection" << std::endl;
            int client_socket = accept(server_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen);
            if (client_socket < 0) {
                std::cout << "Error accepting connection" << std::endl;
            } else {
                std::cout << "accepted connection" << std::endl;
                close(client_socket);
            }
        }
        std::cout << "STOPPED RUNNING" << std::endl;
    });
}
void tcp_blocking_server::stop() {
    std::cout << "STOPPING" << std::endl;
    running = false;
    close(server_socket);
    listen_thread.join();
    std::cout << "STOPPED" << std::endl;
}

int main() {
    // case 1: no connection
    tcp_blocking_server server;
    server.listen();
    usleep(0.5e6); // without the sleep it can stop before it starts the running loop
    std::cout << "STOPPING SERVER" << std::endl;
    server.stop();

    // should get stuck before we even get here

    // case 2: connection
    server.listen();
    tcp_client client;
    client.connect("127.0.0.1", 8888);
    std::cout << "STOPPING SERVER" << std::endl;
    server.stop();
    return 0;
}