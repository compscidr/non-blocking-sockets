//
// Created by jason on 5/26/21.
//

#ifndef MODBUS_TEST_MODBUS_SERVER_H
#define MODBUS_TEST_MODBUS_SERVER_H

#include <atomic>
#include <modbus/modbus.h>
#include <thread>

class modbus_server {
    private:
        modbus_t* context{nullptr};
        int server_socket{-1};
        std::atomic<bool> running{false};
        std::thread listen_thread;
    public:
        modbus_server();
        void listen();
        void stop();
};


#endif//MODBUS_TEST_MODBUS_SERVER_H
