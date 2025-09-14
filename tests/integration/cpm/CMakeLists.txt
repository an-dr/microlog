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
CPMAddPackage(
  NAME microlog
  GIT_REPOSITORY https://github.com/an-dr/microlog.git
  GIT_TAG ${GIT_COMMIT_HASH}
)

add_executable(example_package example.cpp)

# Link against the exported namespace target (preferred) if it exists; fallback to plain target
if(TARGET microlog::microlog)
  target_link_libraries(example_package PRIVATE microlog::microlog)
else()
  target_link_libraries(example_package PRIVATE microlog)
endif()

target_compile_definitions(example_package PRIVATE ULOG_BUILD_COLOR=1)

install(TARGETS example_package)
