cmake_minimum_required(VERSION 3.15)
project(example_package)

include(CPM.cmake)

# If MICROLOG_SOURCE_DIR is set, use the local source tree directly.
# This avoids fetching from a hardcoded upstream URL, so the test works
# correctly from forks and in local development without network access.
if(DEFINED MICROLOG_SOURCE_DIR)
  message(STATUS "CPM: using local source at ${MICROLOG_SOURCE_DIR}")
  cpmaddpackage(NAME microlog SOURCE_DIR "${MICROLOG_SOURCE_DIR}")
else()
  execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  message(STATUS "CPM: fetching microlog commit ${GIT_COMMIT_HASH}")
  cpmaddpackage(NAME microlog
                GIT_REPOSITORY https://github.com/an-dr/microlog.git
                GIT_TAG ${GIT_COMMIT_HASH})
endif()

add_executable(example_package example.cpp)

target_link_libraries(example_package PRIVATE microlog::microlog)
target_compile_definitions(example_package PRIVATE ULOG_BUILD_COLOR=1)

install(TARGETS example_package)
