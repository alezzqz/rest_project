cmake_minimum_required(VERSION 3.4)

project(logger)

find_package(fmt REQUIRED)

set(SOURCES logger.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")

add_library(logger ${SOURCES})

target_link_libraries(${PROJECT_NAME} fmt)