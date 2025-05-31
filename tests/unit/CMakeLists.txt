# CMakeLists.txt for microlog unit tests

# Common sources and include directories
# TODO: Use find_package to locate microlog library?
set(ULOG_SRC ../../src/ulog.c)
set(ULOG_INCLUDE_DIR ../../include)

# Configure test environment
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(--coverage -fprofile-arcs -ftest-coverage -g -O0 -Wall)
add_link_options(--coverage -fprofile-arcs -ftest-coverage)

# Microlog configs
set(ULOG_CONFIG_BASE "-DULOG_EXTRA_OUTPUTS=8" 
                     "-DULOG_HAVE_TIME"
                     "-DULOG_TOPICS_NUM=8"
                     "-DULOG_CUSTOM_PREFIX_SIZE=8"
                     )

                     
# --- Core Tests ---
add_executable(test_core)
target_sources(test_core PRIVATE ${ULOG_SRC} 
                                 ut_callback.c
                                 test_core.cpp)
target_include_directories(test_core PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_core PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME CoreTests COMMAND test_core)
