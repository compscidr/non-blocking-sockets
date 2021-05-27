//
// Created by jason on 5/26/21.
//
#include "modbus_server.h"
#include <iostream>
#include <modbus/modbus.h>
#include <thread>
#include <unistd.h>

modbus_server::modbus_server() {
    context = modbus_new_tcp("localhost", 1502);
}

void modbus_server::listen() {
    listen_thread = std::thread([&]() {
      running = true;
      server_socket = modbus_tcp_listen(context, 1);
      while (running) {
          std::cout << "WAITING FOR CONNECTION" << std::endl;
          int client_socket = modbus_tcp_accept(context, &server_socket);
          if (client_socket == -1) {
              std::cout << "Error on accept" << std::endl;
          } else {
              std::cout << "Got connection" << std::endl;
              close(client_socket);
          }
      }
      std::cout << "Ending listening thread" << std::endl;
    });
}
void modbus_server::stop() {
    std::cout << "stopping listening" << std::endl;
    running = false;
    //close(server_socket);
    modbus_close(context);
    modbus_free(context);

    std::cout << "waiting on listen_thread to stop" << std::endl;
    listen_thread.join();
    std::cout << "done stopping" << std::endl;
}
