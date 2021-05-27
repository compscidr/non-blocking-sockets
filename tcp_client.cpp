//
// Created by jason on 5/26/21.
//

#include "tcp_client.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

tcp_client::tcp_client() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
}

void tcp_client::connect(std::string host, int port) {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    ::inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);
    std::cout << "Connecting..." << std::endl;
    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "failed" << std::endl;
        return;
    }
    std::cout << "connected." << std::endl;
    close(sock);
}
