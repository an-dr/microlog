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
set(ULOG_CONFIG_BASE "-DULOG_BUILD_EXTRA_OUTPUTS=8" 
                     "-DULOG_BUILD_TIME=1"
                     "-DULOG_BUILD_PREFIX_SIZE=16"
                     )

set(ULOG_CONFIG_NO_PREFIX "-DULOG_BUILD_EXTRA_OUTPUTS=8" 
                                "-DULOG_BUILD_TIME=1"
                                "-DULOG_BUILD_TOPICS_NUM=8"
                                )

set(ULOG_CONFIG_TEST_CORE ${ULOG_CONFIG_BASE}
                          "-DULOG_DEFAULT_LOG_LEVEL=ULOG_LEVEL_DEBUG"
                          "-DULOG_BUILD_TOPICS_NUM=-1"
                          )

set(ULOG_CONFIG_TEST_TOPICS ${ULOG_CONFIG_BASE}
                            "-DULOG_DEFAULT_LOG_LEVEL=ULOG_LEVEL_WARN"
                            "-DULOG_BUILD_TOPICS_NUM=2"
                            )

set(ULOG_CONFIG_TEST_DYNAMIC_CONFIG "-DULOG_BUILD_DYNAMIC_CONFIG=1")

# --- Core Tests ---
add_executable(test_core)
target_sources(test_core PRIVATE ${ULOG_SRC} 
                                 ut_callback.c
                                 test_core.cpp)
target_include_directories(test_core PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_core PRIVATE ${ULOG_CONFIG_TEST_CORE})
add_test(NAME CoreTests COMMAND test_core)


# --- Locking Test ---
add_executable(test_locking)
target_sources(test_locking PRIVATE ${ULOG_SRC} 
                                    ut_callback.c 
                                    test_locking.cpp)
target_include_directories(test_locking PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_locking PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME LockingTest COMMAND test_locking)

# --- Prefix Test ---
add_executable(test_prefix)
target_sources(test_prefix PRIVATE ${ULOG_SRC} 
                                          ut_callback.c 
                                          test_prefix.cpp)
target_include_directories(test_prefix PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_prefix PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME CustomPrefixTest COMMAND test_prefix)

# --- Topics Test ---
add_executable(test_topics)
target_sources(test_topics PRIVATE ${ULOG_SRC} 
                                   ut_callback.c 
                                   test_topics.cpp)
target_include_directories(test_topics PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_topics PRIVATE ${ULOG_CONFIG_TEST_TOPICS})
add_test(NAME TopicsTest COMMAND test_topics)

# --- Time Test - With Prefix ---
add_executable(test_time)
target_sources(test_time PRIVATE ${ULOG_SRC} 
                                 ut_callback.c 
                                 test_time.cpp)
target_include_directories(test_time PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_time PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME TimeTest COMMAND test_time)

# --- Time Test - No Prefix ---
add_executable(test_time_np)
target_sources(test_time_np PRIVATE ${ULOG_SRC}     
                                    ut_callback.c 
                                    test_time.cpp)
target_include_directories(test_time_np PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_time_np PRIVATE ${ULOG_CONFIG_NO_PREFIX})
add_test(NAME TimeTestNoPrefix COMMAND test_time_np)

# --- Dynamic Config Test ---
add_executable(test_dynamic_config)
target_sources(test_dynamic_config PRIVATE ${ULOG_SRC} 
                                            ut_callback.c 
                                            test_dynamic_config.cpp)
target_include_directories(test_dynamic_config PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_dynamic_config PRIVATE ${ULOG_CONFIG_TEST_DYNAMIC_CONFIG})
add_test(NAME DynamicConfigTest COMMAND test_dynamic_config)

# --- Event API / Error Paths Test ---
add_executable(test_event_api)
target_sources(test_event_api PRIVATE ${ULOG_SRC}
                                     ut_callback.c
                                     test_event_api.cpp)
target_include_directories(test_event_api PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_event_api PRIVATE ${ULOG_CONFIG_BASE})
add_test(NAME EventApiTest COMMAND test_event_api)

# --- Edge Cases / Error Paths Test ---
add_executable(test_edge_cases)
target_sources(test_edge_cases PRIVATE ${ULOG_SRC}
                                      ut_callback.c
                                      test_edge_cases.cpp)
target_include_directories(test_edge_cases PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_edge_cases PRIVATE ${ULOG_CONFIG_TEST_CORE})
add_test(NAME EdgeCasesTest COMMAND test_edge_cases)

# --- Disabled Features / Warn Not Enabled Test ---
add_executable(test_warn_not_enabled)
target_sources(test_warn_not_enabled PRIVATE ${ULOG_SRC}
                                              ut_callback.c
                                              test_warn_not_enabled.cpp)
target_include_directories(test_warn_not_enabled PRIVATE ${ULOG_INCLUDE_DIR})
# Intentionally disable features to exercise warn_not_enabled stubs
target_compile_definitions(test_warn_not_enabled PRIVATE -DULOG_BUILD_EXTRA_OUTPUTS=0
                                                        -DULOG_BUILD_TIME=0
                                                        -DULOG_BUILD_PREFIX_SIZE=0
                                                        -DULOG_BUILD_TOPICS_NUM=0
                                                        -DULOG_BUILD_WARN_NOT_ENABLED=1)
add_test(NAME WarnNotEnabledTest COMMAND test_warn_not_enabled)

# --- Additional Coverage Test ---
add_executable(test_additional_coverage)
target_sources(test_additional_coverage PRIVATE ${ULOG_SRC}
                                               ut_callback.c
                                               test_additional_coverage.cpp)
target_include_directories(test_additional_coverage PRIVATE ${ULOG_INCLUDE_DIR})
# Need dynamic config for color toggling; reuse BASE plus dynamic config
target_compile_definitions(test_additional_coverage PRIVATE ${ULOG_CONFIG_BASE} ${ULOG_CONFIG_TEST_DYNAMIC_CONFIG} "-DULOG_BUILD_TOPICS_NUM=4")
add_test(NAME AdditionalCoverageTest COMMAND test_additional_coverage)
