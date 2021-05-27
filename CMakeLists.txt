cmake_minimum_required(VERSION 3.10)
project(modbus_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(modbus_test main.cpp modbus_server.cpp modbus_server.h tcp_server.cpp tcp_server.h tcp_client.cpp tcp_client.h)

target_link_libraries(modbus_test modbus pthread)