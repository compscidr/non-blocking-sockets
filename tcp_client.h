//
// Created by jason on 5/26/21.
//

#ifndef MODBUS_TEST_TCP_CLIENT_H
#define MODBUS_TEST_TCP_CLIENT_H


#include <string>
class tcp_client {
private:
    int sock;
public:
    tcp_client();
    void connect(std::string host, int port);
};


#endif//MODBUS_TEST_TCP_CLIENT_H
