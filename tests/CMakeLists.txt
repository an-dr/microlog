cmake_minimum_required(VERSION 3.10)

project(microlog_tests C)

# Add the executable for the test
add_executable(test_runtime_config test_runtime_config.c)

# Link against the microlog library
# Assuming microlog is a target defined in the parent CMakeLists.txt
# and that this CMakeLists.txt is processed via add_subdirectory()
target_link_libraries(test_runtime_config PRIVATE microlog::microlog)

# Include directory for ulog.h from the parent scope
target_include_directories(test_runtime_config PRIVATE ${CMAKE_SOURCE_DIR}/include)

# If CTest is being used in the main project, add this test
if(BUILD_TESTING)
    include(CTest)
    add_test(NAME RuntimeConfigTest COMMAND test_runtime_config)
endif()
