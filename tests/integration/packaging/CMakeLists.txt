cmake_minimum_required(VERSION 3.5)
project(example_package)

find_package(microlog 6.0.0 REQUIRED)

add_compile_options(-DULOG_NO_COLOR)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DULOG_NO_COLOR")
add_executable(example_package example.cpp)
target_link_libraries(example_package PRIVATE microlog::microlog)

install(TARGETS example_package)
