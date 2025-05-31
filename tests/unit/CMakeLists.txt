# CMakeLists.txt for microlog unit tests

# Common sources and include directories
set(MICROLOG_SRC ../../src/ulog.c)
set(MICROLOG_INCLUDE_DIR ../../include)

# --- Core Tests ---
add_executable(test_core)
target_sources(test_core PRIVATE ${MICROLOG_SRC} ut_callback.c
                                 test_core.cpp)
target_include_directories(test_core PRIVATE ${MICROLOG_INCLUDE_DIR})
target_compile_definitions(test_core PRIVATE "ULOG_EXTRA_OUTPUTS=1")

add_test(NAME CoreTests COMMAND test_core)

# --- Locking Test ---
add_executable(test_locking)
target_sources(test_locking PRIVATE ${MICROLOG_SRC} ut_callback.c test_locking.cpp)
target_include_directories(test_locking PRIVATE ${MICROLOG_INCLUDE_DIR})
target_compile_definitions(test_locking PRIVATE "ULOG_EXTRA_OUTPUTS=1")
add_test(NAME LockingTest COMMAND test_locking)

# --- Color Test ---
add_executable(test_color)
target_sources(test_color PRIVATE ${MICROLOG_SRC} ut_callback.c test_color.cpp)
target_include_directories(test_color PRIVATE ${MICROLOG_INCLUDE_DIR})
target_compile_definitions(test_color PRIVATE "ULOG_EXTRA_OUTPUTS=1")
add_test(NAME ColorTest COMMAND test_color)

# --- Time Test ---
add_executable(test_time)
target_sources(test_time PRIVATE ${MICROLOG_SRC} ut_callback.c test_time.cpp)
target_include_directories(test_time PRIVATE ${MICROLOG_INCLUDE_DIR})
target_compile_definitions(test_time PRIVATE "ULOG_EXTRA_OUTPUTS=1")
add_test(NAME TimeTest COMMAND test_time)

# --- Custom Prefix Test ---
add_executable(test_custom_prefix)
target_sources(test_custom_prefix PRIVATE ${MICROLOG_SRC} ut_callback.c test_custom_prefix.cpp)
target_include_directories(test_custom_prefix PRIVATE ${MICROLOG_INCLUDE_DIR})
target_compile_definitions(test_custom_prefix PRIVATE "ULOG_EXTRA_OUTPUTS=1")
add_test(NAME CustomPrefixTest COMMAND test_custom_prefix)

# --- Topics Test ---
add_executable(test_topics)
target_sources(test_topics PRIVATE ${MICROLOG_SRC} ut_callback.c test_topics.cpp)
target_include_directories(test_topics PRIVATE ${MICROLOG_INCLUDE_DIR})
target_compile_definitions(test_topics PRIVATE "ULOG_EXTRA_OUTPUTS=1")
add_test(NAME TopicsTest COMMAND test_topics)
