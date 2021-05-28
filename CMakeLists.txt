cmake_minimum_required(VERSION 3.10)
project(modbus_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(modbus_test modbus_server.cpp modbus_server.h)
add_executable(tcp_test tcp_server.cpp tcp_server.h tcp_client.cpp tcp_client.h)
add_executable(tcp_blocking_test tcp_blocking_server.cpp tcp_blocking_server.h tcp_client.cpp tcp_client.h)

target_link_libraries(modbus_test modbus pthread)
target_link_libraries(tcp_test modbus pthread)
target_link_libraries(tcp_blocking_test modbus pthread)