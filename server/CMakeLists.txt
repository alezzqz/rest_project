cmake_minimum_required(VERSION 3.4)

project(cmserver)

find_package(Boost 1.66 REQUIRED COMPONENTS system)

set(Boost_USE_STATIC_LIBS ON)
set(SOURCES api_router.cpp api_server.cpp main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")

add_executable(cmserver ${SOURCES})

target_link_libraries(${PROJECT_NAME} boost_system)

install(TARGETS cmserver DESTINATION /usr/sbin)
install(FILES cmserver.service DESTINATION /etc/systemd/system)