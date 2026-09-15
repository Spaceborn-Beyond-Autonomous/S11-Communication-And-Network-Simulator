cmake_minimum_required(VERSION 3.16)
project(S11CommunicationAndNetworkSimulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(check_1
    tests/check_1.c
    src/topology/link_manager.c
    src/logger/network_logger.c
)
