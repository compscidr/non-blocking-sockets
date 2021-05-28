//
// Created by jason on 5/27/21.
//

#ifndef MODBUS_TEST_TCP_BLOCKING_SERVER_H
#define MODBUS_TEST_TCP_BLOCKING_SERVER_H

#include <atomic>
#include <thread>

class tcp_blocking_server {
private:
    int server_socket{-1};
    std::atomic<bool> running{false};
    std::thread listen_thread;
public:
    tcp_blocking_server();
    static void errExit(std::string err);
    void listen();
    void stop();
};


#endif//MODBUS_TEST_TCP_BLOCKING_SERVER_H
