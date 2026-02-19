cmake_minimum_required(VERSION 3.15)
project(example_vcpkg C)

find_package(microlog CONFIG REQUIRED)

add_executable(example_vcpkg example.c)
target_link_libraries(example_vcpkg PRIVATE microlog::microlog)

install(TARGETS example_vcpkg)
