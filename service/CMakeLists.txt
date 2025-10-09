cmake_minimum_required(VERSION 3.4)

project(service)

set(SOURCES stop_signal.cpp pid_file.cpp service.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")

add_library(service ${SOURCES})
