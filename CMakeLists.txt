cmake_minimum_required(VERSION 3.5)
project(microlog)

add_library(microlog STATIC src/ulog.c)
target_include_directories(microlog PUBLIC include)

install(TARGETS microlog DESTINATION lib)
