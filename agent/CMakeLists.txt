cmake_minimum_required(VERSION 3.4)

project(cmagent)

find_package(Boost 1.66 REQUIRED COMPONENTS system program_options)
find_package(yaml-cpp REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(SOURCES config.cpp main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")

add_executable(cmagent ${SOURCES})

target_link_libraries(${PROJECT_NAME} boost_system boost_program_options logger service restapi yaml-cpp)

file(COPY cmagent.yaml DESTINATION ${CMAKE_BINARY_DIR}/server/)

install(TARGETS cmagent DESTINATION /usr/sbin)
install(FILES cmagent.service DESTINATION /etc/systemd/system)
install(FILES cmagent.yaml DESTINATION /etc/cmagent)