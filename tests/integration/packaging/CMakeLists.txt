cmake_minimum_required(VERSION 3.15)
project(example_package)

find_package(microlog 7.0.0 REQUIRED)

add_executable(example_package example.cpp)

target_link_libraries(example_package PRIVATE microlog::microlog)

install(TARGETS example_package)
