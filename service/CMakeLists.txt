cmake_minimum_required(VERSION 3.4)

project(service)

# find_package(Boost 1.66 REQUIRED COMPONENTS system)

# set(Boost_USE_STATIC_LIBS ON)
set(SOURCES stop_signal.cpp pid_file.cpp service.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")

add_library(service ${SOURCES})

#target_link_libraries(${PROJECT_NAME} boost_system)
