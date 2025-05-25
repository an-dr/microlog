# CMakeLists.txt for microlog unit tests

# Common sources and include directories
set(MICROLOG_SRC ../../src/ulog.c)
set(MICROLOG_INCLUDE_DIR ../../include)

# --- Core Tests ---
add_executable(test_core test_core.c ${MICROLOG_SRC})
target_include_directories(test_core PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
add_test(NAME CoreTests COMMAND test_core)

# --- Formatting Tests ---

# Default formatting test
add_executable(test_formatting_default test_formatting.c ${MICROLOG_SRC})
target_include_directories(test_formatting_default PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
add_test(NAME FormattingDefault COMMAND test_formatting_default)

# Test with ULOG_HAVE_TIME
add_executable(test_formatting_time test_formatting.c ${MICROLOG_SRC})
target_include_directories(test_formatting_time PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
target_compile_definitions(test_formatting_time PRIVATE ULOG_HAVE_TIME)
add_test(NAME FormattingTime COMMAND test_formatting_time)

# Test with ULOG_HIDE_FILE_STRING
add_executable(test_formatting_no_file_str test_formatting.c ${MICROLOG_SRC})
target_include_directories(test_formatting_no_file_str PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
target_compile_definitions(test_formatting_no_file_str PRIVATE ULOG_HIDE_FILE_STRING)
add_test(NAME FormattingNoFileString COMMAND test_formatting_no_file_str)

# Test with ULOG_SHORT_LEVEL_STRINGS
add_executable(test_formatting_short_level test_formatting.c ${MICROLOG_SRC})
target_include_directories(test_formatting_short_level PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
target_compile_definitions(test_formatting_short_level PRIVATE ULOG_SHORT_LEVEL_STRINGS)
add_test(NAME FormattingShortLevel COMMAND test_formatting_short_level)

# Test with ULOG_USE_EMOJI
add_executable(test_formatting_emoji_level test_formatting.c ${MICROLOG_SRC})
target_include_directories(test_formatting_emoji_level PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
target_compile_definitions(test_formatting_emoji_level PRIVATE ULOG_USE_EMOJI)
add_test(NAME FormattingEmojiLevel COMMAND test_formatting_emoji_level)

# --- Prefix Test ---
# For test_prefix.c
add_executable(test_prefix test_prefix.c ${MICROLOG_SRC})
target_include_directories(test_prefix PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
# Define ULOG_CUSTOM_PREFIX_SIZE to enable the feature for this test and set its size
target_compile_definitions(test_prefix PRIVATE "ULOG_CUSTOM_PREFIX_SIZE=16")
add_test(NAME PrefixTests COMMAND test_prefix)

# --- Extra Outputs Test ---
# For test_extra_outputs.c
add_executable(test_extra_outputs test_extra_outputs.c ${MICROLOG_SRC})
target_include_directories(test_extra_outputs PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
# Define ULOG_EXTRA_OUTPUTS to enable the feature for this test and set its size
target_compile_definitions(test_extra_outputs PRIVATE "ULOG_EXTRA_OUTPUTS=2")
add_test(NAME ExtraOutputsTests COMMAND test_extra_outputs)

# --- Topic Tests ---
# For test_topics.c

# Static topic allocation test
add_executable(test_topics_static test_topics.c ${MICROLOG_SRC})
target_include_directories(test_topics_static PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
target_compile_definitions(test_topics_static PRIVATE 
    "ULOG_TOPICS_NUM=5"       # Static allocation with 5 topic slots
    "ULOG_EXTRA_OUTPUTS=1"    # Need at least one extra output for the test callback
)
add_test(NAME TopicTestsStatic COMMAND test_topics_static)

# Dynamic topic allocation test
add_executable(test_topics_dynamic test_topics.c ${MICROLOG_SRC})
target_include_directories(test_topics_dynamic PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
target_compile_definitions(test_topics_dynamic PRIVATE 
    "ULOG_TOPICS_NUM=-1"      # Dynamic allocation
    "ULOG_EXTRA_OUTPUTS=1"    # Need at least one extra output for the test callback
)
add_test(NAME TopicTestsDynamic COMMAND test_topics_dynamic)

# --- Thread Safety Test (Locking) ---
# For test_thread_safety.c
add_executable(test_thread_safety test_thread_safety.c ${MICROLOG_SRC})
target_include_directories(test_thread_safety PRIVATE ${MICROLOG_INCLUDE_DIR} ../../src)
# ULOG_EXTRA_OUTPUTS is included for consistency, though not strictly used by this specific lock test.
target_compile_definitions(test_thread_safety PRIVATE "ULOG_EXTRA_OUTPUTS=1")
add_test(NAME ThreadSafetyLockTest COMMAND test_thread_safety)
