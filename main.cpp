#include <iostream>
#include "modbus_server.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include <unistd.h>

int main() {
//    modbus_server server;
    tcp_server server;
    tcp_client client;
    server.listen();
    //usleep(0.5e6);
    client.connect("127.0.0.1", 8888);
    //usleep(0.5e6);
    std::cout << "STOPPING SERVER" << std::endl;
    server.stop();
    //usleep(0.5e6);
    return 0;
}
