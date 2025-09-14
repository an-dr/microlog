cmake_minimum_required(VERSION 3.15)
project(example_package)

if(DEFINED ENV{GITHUB_SHA})
  set(GIT_COMMIT_HASH "$ENV{GITHUB_SHA}")
else()
  # Fallback: query Git directly
  execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()
message(STATUS "Current commit: ${GIT_COMMIT_HASH}")

include(CPM.cmake)
cpmaddpackage(NAME microlog GIT_REPOSITORY
              https://github.com/an-dr/microlog.git GIT_TAG ${GIT_COMMIT_HASH})

add_executable(example_package example.cpp)

target_link_libraries(example_package PRIVATE microlog::microlog)
target_compile_definitions(example_package PRIVATE ULOG_BUILD_COLOR=1)

install(TARGETS example_package)
