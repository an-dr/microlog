# CMakeLists.txt for microlog unit tests

# Common sources and include directories
set(ULOG_SRC ../../src/ulog.c)
set(ULOG_INCLUDE_DIR ../../include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configs
set(ULOG_CONFIG_BASE_LIST "-DULOG_EXTRA_OUTPUTS=8" 
                     "-DULOG_HAVE_TIME"
                     "-DULOG_TOPICS_NUM=8"
                     "-DULOG_CUSTOM_PREFIX_SIZE=8"
                     )

                     
# --- Core Tests ---
add_executable(test_core)
target_sources(test_core PRIVATE ${ULOG_SRC} ut_callback.c
                                 test_core.cpp)
target_include_directories(test_core PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_core PRIVATE ${ULOG_CONFIG_BASE_LIST})

# Add coverage flags if COVERAGE is ON
# option(COVERAGE "Enable coverage reporting" OFF)
# if(COVERAGE)
    message(STATUS "Building with coverage flags")
    target_compile_options(test_core PRIVATE --coverage -fprofile-arcs -ftest-coverage -g -O0 -Wall)
    target_link_libraries(test_core PRIVATE --coverage -fprofile-arcs -ftest-coverage)
# endif()

add_test(NAME CoreTests COMMAND test_core)

# https://github.com/jhbell/cmake-gcov/blob/master/CMakeLists.txt
