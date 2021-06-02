//
// Created by jason on 5/26/21.
//
#include "modbus_server.h"
#include <asm-generic/ioctls.h>
#include <asm-generic/socket.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <modbus/modbus.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

void modbus_server::errExit(std::string s) {
    std::cout << s << std::endl;
    exit(1);
}

modbus_server::modbus_server() {
}

void modbus_server::init() {
    context = modbus_new_tcp("127.0.0.1", 1502);
    server_socket = modbus_tcp_listen(context, 1);
    FD_ZERO(&readfds);

    if (pipe(modbus_server::pfd) == -1)
        errExit("pipe");

    FD_SET(modbus_server::pfd[0], &readfds);           /* Add read end of pipe to 'readfds' */
    nfds = std::max(nfds, modbus_server::pfd[0] + 1);            /* And adjust 'nfds' if required */
    std::cout << "PFD: " << pfd[0] << " NFDS: " << nfds << std::endl;

    /* Make read and write ends of pipe nonblocking */
    int flags = fcntl(modbus_server::pfd[0], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    flags |= O_NONBLOCK;                /* Make read end nonblocking */
    if (fcntl(modbus_server::pfd[0], F_SETFL, flags) == -1)
        errExit("fcntl-F_SETFL");

    flags = fcntl(modbus_server::pfd[1], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    flags |= O_NONBLOCK;                /* Make write end nonblocking */
    if (fcntl(modbus_server::pfd[1], F_SETFL, flags) == -1)
        errExit("fcntl-F_SETFL");

    // register the signal handlers
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;           /* Restart interrupted reads()s */
    sa.sa_handler = handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        errExit("sigaction");

    FD_SET(server_socket, &readfds);           /* Add server socket to 'readfds' */
    nfds = std::max(nfds, server_socket + 1);         /* And adjust 'nfds' if required */
    std::cout << "SS: " << server_socket << " NFDS: " << nfds << std::endl;
}

void modbus_server::listen() {
    listen_thread = std::thread([&]() {
      running = true;
      int client_socket = -1;
      while (running) {
          std::cout << "WAITING FOR CONNECTION" << std::endl;
          int ready;
          struct timeval *pto = NULL;
          // need to use a working set because after select is done, it screws up the original
          fd_set working_set;
          memcpy(&working_set, &readfds, sizeof(readfds));
          while ((ready = select(nfds, &working_set, NULL, NULL, pto)) == -1 &&
                 errno == EINTR) {
              std::cout << "L" << std::endl;
              continue; /* Restart if interrupted by signal */
          }
          if (ready == -1)                    /* Unexpected error */
              errExit("select");

          if (FD_ISSET(pfd[0], &working_set)) {   /* Handler was called */
              printf("A signal was caught\n");

              for (;;) {                      /* Consume bytes from pipe */
                  char ch;
                  if (read(pfd[0], &ch, 1) == -1) {
                      if (errno == EAGAIN)
                          break;              /* No more bytes */
                      else
                          errExit("read");    /* Some other error */
                  }

                  /* Perform any actions that should be taken in response to signal */
                  return;
              }
          }

          if (FD_ISSET(server_socket, &working_set)) {
              std::cout << "readfds server socket ready" << std::endl;
              client_socket = modbus_tcp_accept(context, &server_socket);
              if (client_socket == -1) {
                  std::cout << "Error on accept" << std::endl;
              } else {
                  std::cout << "Got connection" << std::endl;
                  FD_SET(client_socket, &readfds);
                  nfds = std::max(nfds, client_socket + 1); /* And adjust 'nfds' if required */
                  //close(client_socket);
              }
          }

          if (client_socket > 0 && FD_ISSET(client_socket, &working_set)) {
              uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH]{};
              int32_t receive_size = modbus_receive(context, query);
              std::cout << "received: " << receive_size << " bytes" << std::endl;
              modbus_mapping_t *mapping = modbus_mapping_new(10, 10, 10, 10);
              modbus_reply(context, query, receive_size, mapping);
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
    raise(SIGUSR1);
    listen_thread.join();
    std::cout << "done stopping" << std::endl;
}

void modbus_server::handler(int sig)
{
    std::cout << "IN SIG HANDLER" << std::endl;
    int savedErrno;                     /* In case we change 'errno' */

    savedErrno = errno;
    if (write(pfd[1], "x", 1) == -1 && errno != EAGAIN)
        errExit("write");

    std::cout << "Wrote to pipe" << std::endl;
    errno = savedErrno;
}

int main() {
    // case 1: no connection
    modbus_server server;
//    server.init();
//    server.listen();
//    usleep(0.5e6); // without the sleep it can stop before it starts the running loop
//    std::cout << "STOPPING SERVER" << std::endl;
//    server.stop();

    // case 2: connection
    std::cout << "-----------------------------" << std::endl;
    std::cout << "STARTING AGAIN" << std::endl;
    server.init();
    server.listen();
    usleep(0.5e6); // without the sleep it can stop before it starts the running loop

    auto context = modbus_new_tcp("127.0.0.1", 1502);
    if (modbus_connect(context) == -1) {
        std::cout << "connection failed" << std::endl;
    } else {
        std::cout << "connected" << std::endl;
    }

    auto result = modbus_write_bit(context, 0, true);
    if (result == -1) {
        std::cout << "write failed" << std::endl;
        switch (errno) {
            case EMBXILADD:
                std::cout << "Address error when trying to set a coil on a modbus client" << std::endl;
                break;
            case EMBXILVAL:
                std::cout << "Value error when trying to set a coil on a modbus client" << std::endl;
                break;
            default:
                std::cout << "Server error when trying to set a coil on a modbus client, disconnecting: " << modbus_strerror(errno) << std::endl;
        }
    } else {
        std::cout << "write success" << std::endl;
    }

    modbus_free(context);
    std::cout << "STOPPING SERVER" << std::endl;
    server.stop();

    return 0;
}