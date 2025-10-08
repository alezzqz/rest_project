cmake_minimum_required(VERSION 3.2)

project(cm)

add_subdirectory(service)
add_subdirectory(restapi)
add_subdirectory(server)

target_include_directories(service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(restapi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})