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
                     "-DULOG_CUSTOM_PREFIX_SIZE=16"
                     )

                     
# --- Core Tests ---
add_executable(test_core)
target_sources(test_core PRIVATE ${ULOG_SRC} 
                                 ut_callback.c
                                 test_core.cpp)
target_include_directories(test_core PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_core PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME CoreTests COMMAND test_core)


# --- Locking Test ---
add_executable(test_locking)
target_sources(test_locking PRIVATE ${ULOG_SRC} 
                                    ut_callback.c 
                                    test_locking.cpp)
target_include_directories(test_locking PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_locking PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME LockingTest COMMAND test_locking)

# --- Custom Prefix Test ---
add_executable(test_custom_prefix)
target_sources(test_custom_prefix PRIVATE ${ULOG_SRC} 
                                          ut_callback.c 
                                          test_custom_prefix.cpp)
target_include_directories(test_custom_prefix PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_custom_prefix PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME CustomPrefixTest COMMAND test_custom_prefix)

# --- Topics Test ---
add_executable(test_topics)
target_sources(test_topics PRIVATE ${ULOG_SRC} 
                                   ut_callback.c 
                                   test_topics.cpp)
target_include_directories(test_topics PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_topics PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME TopicsTest COMMAND test_topics)
