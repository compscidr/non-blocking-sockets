//
// Created by jason on 5/26/21.
//
#include "tcp_server.h"
#include <asm-generic/ioctls.h>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

void errExit(std::string s) {
    std::cout << s << std::endl;
    exit(1);
}

// https://www.geeksforgeeks.org/socket-programming-cc/
// https://www.ibm.com/docs/en/i/7.2?topic=designs-example-nonblocking-io-select
tcp_server::tcp_server() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        errExit("resuable");
    }
    if (ioctl(server_socket, FIONBIO, &opt) < 0) {
        errExit("non-blocking");
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( 8888);
    if (bind(server_socket, (struct sockaddr *)&address,sizeof(address)) < 0) {
        errExit("bind");
    } else {
        std::cout << "bound to port: " << ntohs(address.sin_port) << std::endl;
    }
    FD_ZERO(&readfds);

    if (pipe(tcp_server::pfd) == -1)
        errExit("pipe");

    FD_SET(tcp_server::pfd[0], &readfds);           /* Add read end of pipe to 'readfds' */
    nfds = std::max(nfds, tcp_server::pfd[0] + 1);            /* And adjust 'nfds' if required */
    std::cout << "PFD: " << pfd[0] << " NFDS: " << nfds << std::endl;

    /* Make read and write ends of pipe nonblocking */
    int flags = fcntl(tcp_server::pfd[0], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    flags |= O_NONBLOCK;                /* Make read end nonblocking */
    if (fcntl(tcp_server::pfd[0], F_SETFL, flags) == -1)
        errExit("fcntl-F_SETFL");

    flags = fcntl(tcp_server::pfd[1], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    flags |= O_NONBLOCK;                /* Make write end nonblocking */
    if (fcntl(tcp_server::pfd[1], F_SETFL, flags) == -1)
        errExit("fcntl-F_SETFL");

    // register the signal handlers
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;           /* Restart interrupted reads()s */
    sa.sa_handler = handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        errExit("sigaction");
}

void tcp_server::listen() {
    FD_SET(server_socket, &readfds);           /* Add server socket to 'readfds' */
    nfds = std::max(nfds, server_socket + 1);         /* And adjust 'nfds' if required */
    std::cout << "SS: " << server_socket << " NFDS: " << nfds << std::endl;

    if(::listen(server_socket, 3) < 0) {
        errExit("listen");
    }
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    listen_thread = std::thread([&]() {
        running = true;
        std::cout << "Listening" << std::endl;
        while(running) {
            int ready;
            struct timeval *pto = NULL;
            while ((ready = select(nfds, &readfds, NULL, NULL, pto)) == -1 &&
                   errno == EINTR) {
                std::cout << "L" << std::endl;
                continue; /* Restart if interrupted by signal */
            }
            if (ready == -1)                    /* Unexpected error */
                errExit("select");

            if (FD_ISSET(pfd[0], &readfds)) {   /* Handler was called */
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

            if (FD_ISSET(server_socket, &readfds)) {
                std::cout << "readfds server socket ready" << std::endl;
                int client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                if (client_socket < 0) {
                    std::cout << "Error accepting connection";
                } else {
                    std::cout << "accepted connection" << std::endl;
                    close(client_socket);
                }
            }
        }
        std::cout << "Done listening thread" << std::endl;
    });
}

void tcp_server::handler(int sig)
{
    std::cout << "IN SIG HANDLER" << std::endl;
    int savedErrno;                     /* In case we change 'errno' */

    savedErrno = errno;
    if (write(pfd[1], "x", 1) == -1 && errno != EAGAIN)
        errExit("write");

    errno = savedErrno;
}

void tcp_server::stop() {
    std::cout << "stopping listening" << std::endl;
    running = false;
    FD_CLR(server_socket, &readfds);
    close(server_socket);

    std::cout << "waiting on listen_thread to stop" << std::endl;
    raise(SIGUSR1);
    listen_thread.join();
    std::cout << "done stopping" << std::endl;
}
