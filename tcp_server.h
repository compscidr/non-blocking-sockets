//
// Created by jason on 5/26/21.
//

#ifndef MODBUS_TEST_TCP_SERVER_H
#define MODBUS_TEST_TCP_SERVER_H

#include <atomic>
#include <thread>

class tcp_server {
private:
    int server_socket{-1};
    std::atomic<bool> running{false};
    std::thread listen_thread;
    fd_set readfds;
    int nfds{0};
    inline static int pfd[2]{0,0}; // https://man7.org/tlpi/code/online/dist/altio/self_pipe.c.html
public:
    tcp_server();
    static void errExit(std::string err);
    void init();
    void listen();
    void stop();
    static void handler(int sig);
};


#endif//MODBUS_TEST_TCP_SERVER_H
