cmake_minimum_required(VERSION 3.15)
project(example_package)


execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Current commit: ${GIT_COMMIT_HASH}")


include(CPM.cmake)
CPMAddPackage(    NAME microlog
  GIT_REPOSITORY https://github.com/an-dr/microlog.git
  GIT_TAG ${GIT_COMMIT_HASH}
)

target_link_libraries(${PROJECT_NAME} PUBLIC microlog)
target_compile_definitions( microlog
        INTERFACE
        ULOG_BUILD_COLOR=1) # configuration


add_executable(example_package example.cpp)

target_link_libraries(example_package PRIVATE microlog::microlog)

install(TARGETS example_package)
