cmake_minimum_required(VERSION 3.5)
project(microlog)

include(${CMAKE_CURRENT_LIST_DIR}/ab.cmake)
add_component(microlog INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
