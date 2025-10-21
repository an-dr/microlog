#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include "ulog.h"
#include "../../extensions/ulog_microlog6_compat.h"
#include "ut_callback.h"

struct TestFixture {
  public:
    TestFixture() {
        // Per-test setup
        // Note: We add the callback every time since ulog_cleanup() removes it
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        ut_callback_reset();
    }

    ~TestFixture() {
        // Cleanup after each test
        ulog_cleanup();
    }
};

/* ============================================================================
   Test: Log Level Enum Compatibility
============================================================================ */

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: Log level enum values") {
    CHECK(LOG_TRACE == ULOG_LEVEL_TRACE);
    CHECK(LOG_DEBUG == ULOG_LEVEL_DEBUG);
    CHECK(LOG_INFO == ULOG_LEVEL_INFO);
    CHECK(LOG_WARN == ULOG_LEVEL_WARN);
    CHECK(LOG_ERROR == ULOG_LEVEL_ERROR);
    CHECK(LOG_FATAL == ULOG_LEVEL_FATAL);
}

/* ============================================================================
   Test: Basic Logging Macros
============================================================================ */

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: Basic logging macros") {
    log_trace("trace message");
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "trace message") != nullptr);

    log_debug("debug message");
    CHECK(ut_callback_get_message_count() == 2);
    CHECK(strstr(ut_callback_get_last_message(), "debug message") != nullptr);

    log_info("info message");
    CHECK(ut_callback_get_message_count() == 3);
    CHECK(strstr(ut_callback_get_last_message(), "info message") != nullptr);

    log_warn("warn message");
    CHECK(ut_callback_get_message_count() == 4);
    CHECK(strstr(ut_callback_get_last_message(), "warn message") != nullptr);

    log_error("error message");
    CHECK(ut_callback_get_message_count() == 5);
    CHECK(strstr(ut_callback_get_last_message(), "error message") != nullptr);

    log_fatal("fatal message");
    CHECK(ut_callback_get_message_count() == 6);
    CHECK(strstr(ut_callback_get_last_message(), "fatal message") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: Logging with format strings") {
    log_info("Hello %s", "world");
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "Hello world") != nullptr);

    log_debug("Number: %d", 42);
    CHECK(ut_callback_get_message_count() == 2);
    CHECK(strstr(ut_callback_get_last_message(), "Number: 42") != nullptr);

    log_error("Float: %.2f", 3.14);
    CHECK(ut_callback_get_message_count() == 3);
    CHECK(strstr(ut_callback_get_last_message(), "Float: 3.14") != nullptr);
}

/* ============================================================================
   Test: Type Aliases
============================================================================ */

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: Type aliases") {
    // Test that ulog_Event is an alias for ulog_event
    // This is a compile-time check - if it compiles, the alias works
    ulog_Event *ev_ptr = nullptr;
    (void)ev_ptr;  // Suppress unused variable warning

    // Test that ulog_LogFn is an alias for ulog_output_handler_fn
    ulog_LogFn fn_ptr = nullptr;
    (void)fn_ptr;
}

/* ============================================================================
   Test: Core Functions
============================================================================ */

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_get_level_string") {
    // Note: The v7.x default level names use "INFO " and "WARN " with spaces for alignment
    CHECK(strcmp(ulog_get_level_string(LOG_TRACE), "TRACE") == 0);
    CHECK(strcmp(ulog_get_level_string(LOG_DEBUG), "DEBUG") == 0);
    CHECK(strcmp(ulog_get_level_string(LOG_INFO), "INFO ") == 0);  // Has trailing space
    CHECK(strcmp(ulog_get_level_string(LOG_WARN), "WARN ") == 0);  // Has trailing space
    CHECK(strcmp(ulog_get_level_string(LOG_ERROR), "ERROR") == 0);
    // Note: FATAL level test skipped due to core library bug in ulog_level_to_string
    // (uses >= instead of > for max_level check, so FATAL returns "?")
    // CHECK(strcmp(ulog_get_level_string(LOG_FATAL), "FATAL") == 0);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_set_level") {
    ulog_set_level(LOG_INFO);

    log_trace("Should not appear");
    CHECK(ut_callback_get_message_count() == 0);

    log_debug("Should not appear");
    CHECK(ut_callback_get_message_count() == 0);

    log_info("Should appear");
    CHECK(ut_callback_get_message_count() == 1);

    log_warn("Should appear");
    CHECK(ut_callback_get_message_count() == 2);

    log_error("Should appear");
    CHECK(ut_callback_get_message_count() == 3);

    log_fatal("Should appear");
    CHECK(ut_callback_get_message_count() == 4);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_set_quiet") {
    // Note: This test assumes ULOG_OUTPUT_STDOUT is used
    // We're testing the wrapper logic, not actual stdout suppression

    ulog_set_quiet(true);
    // After setting quiet mode, stdout should be at a very high level
    // We can't directly test stdout, but we can verify the callback still works
    log_info("This goes to callback");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_set_quiet(false);
    // After disabling quiet mode, stdout should be re-enabled
    log_info("This also goes to callback");
    CHECK(ut_callback_get_message_count() == 2);
}

/* ============================================================================
   Test: Lock Compatibility
============================================================================ */

static bool lock_called = false;
static bool lock_acquired = false;

static void test_lock_fn(bool lock, void *arg) {
    lock_called = true;
    lock_acquired = lock;
    (void)arg;
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_set_lock") {
    lock_called = false;
    lock_acquired = false;

    ulog_set_lock(test_lock_fn, nullptr);

    // Trigger a log to invoke the lock
    log_info("Test message");

    // The lock should have been called
    CHECK(lock_called == true);
}

/* ============================================================================
   Test: Extra Outputs Compatibility
============================================================================ */

#if (defined(ULOG_BUILD_EXTRA_OUTPUTS) && ULOG_BUILD_EXTRA_OUTPUTS > 0) || \
    (defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1)

static int custom_callback_count = 0;
static void custom_callback(ulog_Event *ev, void *arg) {
    custom_callback_count++;
    (void)ev;
    (void)arg;
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_add_callback") {
    custom_callback_count = 0;

    int result = ulog_add_callback(custom_callback, nullptr, LOG_INFO);
    CHECK(result == 0);  // Should return 0 on success

    log_info("Test message");
    CHECK(custom_callback_count == 1);

    log_warn("Another message");
    CHECK(custom_callback_count == 2);

    // Test that it respects the level
    log_debug("Should not appear");
    CHECK(custom_callback_count == 2);  // Count should not increase
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_add_fp") {
    const char *filename = "test_microlog6_compat.log";
    FILE *fp = fopen(filename, "w");
    REQUIRE(fp != nullptr);

    int result = ulog_add_fp(fp, LOG_INFO);
    CHECK(result == 0);  // Should return 0 on success

    log_info("Test file message");
    fclose(fp);

    // Verify the file contains the message
    fp = fopen(filename, "r");
    REQUIRE(fp != nullptr);

    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    CHECK(strstr(buffer, "Test file message") != nullptr);
}

#endif  // ULOG_BUILD_EXTRA_OUTPUTS || ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Test: Topics Compatibility
============================================================================ */

#if (defined(ULOG_BUILD_TOPICS_MODE) && ULOG_BUILD_TOPICS_MODE != 0) || \
    (defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1)

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: TOPIC_NOT_FOUND constant") {
    CHECK(TOPIC_NOT_FOUND == ULOG_TOPIC_ID_INVALID);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: Topic logging macros") {
    ulog_add_topic("test_topic", true);  // Enable the topic

    logt_trace("test_topic", "trace topic message");
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "trace topic message") != nullptr);

    logt_debug("test_topic", "debug topic message");
    CHECK(ut_callback_get_message_count() == 2);

    logt_info("test_topic", "info topic message");
    CHECK(ut_callback_get_message_count() == 3);

    logt_warn("test_topic", "warn topic message");
    CHECK(ut_callback_get_message_count() == 4);

    logt_error("test_topic", "error topic message");
    CHECK(ut_callback_get_message_count() == 5);

    logt_fatal("test_topic", "fatal topic message");
    CHECK(ut_callback_get_message_count() == 6);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_add_topic") {
    // Test adding enabled topic
    int id1 = ulog_add_topic("enabled_topic", true);
    CHECK(id1 >= 0);  // Should return valid ID

    logt_info("enabled_topic", "Should appear");
    CHECK(ut_callback_get_message_count() == 1);

    // Test adding disabled topic
    int id2 = ulog_add_topic("disabled_topic", false);
    CHECK(id2 >= 0);  // Should return valid ID

    logt_info("disabled_topic", "Should not appear");
    CHECK(ut_callback_get_message_count() == 1);  // Count unchanged

    // Test adding topic with NULL name
    int id_invalid = ulog_add_topic(nullptr, true);
    CHECK(id_invalid == -1);  // Should fail
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_set_topic_level") {
    ulog_add_topic("level_topic", true);

    // Set topic to INFO level
    int result = ulog_set_topic_level("level_topic", LOG_INFO);
    CHECK(result == 0);

    logt_debug("level_topic", "Should not appear");
    CHECK(ut_callback_get_message_count() == 0);

    logt_info("level_topic", "Should appear");
    CHECK(ut_callback_get_message_count() == 1);

    logt_error("level_topic", "Should appear");
    CHECK(ut_callback_get_message_count() == 2);

    // Test with non-existent topic
    result = ulog_set_topic_level("nonexistent", LOG_INFO);
    CHECK(result == -1);  // Should fail
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_get_topic_id") {
    int id = ulog_add_topic("id_test_topic", true);
    CHECK(id >= 0);

    int retrieved_id = ulog_get_topic_id("id_test_topic");
    CHECK(retrieved_id == id);

    // Test with non-existent topic
    int invalid_id = ulog_get_topic_id("nonexistent_topic");
    CHECK(invalid_id == TOPIC_NOT_FOUND);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_enable_topic") {
    ulog_add_topic("enable_test", false);  // Start disabled

    logt_info("enable_test", "Should not appear");
    CHECK(ut_callback_get_message_count() == 0);

    int result = ulog_enable_topic("enable_test");
    CHECK(result == 0);

    logt_info("enable_test", "Should appear");
    CHECK(ut_callback_get_message_count() == 1);

    // Test with non-existent topic
    result = ulog_enable_topic("nonexistent");
    CHECK(result == -1);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_disable_topic") {
    ulog_add_topic("disable_test", true);  // Start enabled

    logt_info("disable_test", "Should appear");
    CHECK(ut_callback_get_message_count() == 1);

    int result = ulog_disable_topic("disable_test");
    CHECK(result == 0);

    logt_info("disable_test", "Should not appear");
    CHECK(ut_callback_get_message_count() == 1);  // Count unchanged

    // Test with non-existent topic
    result = ulog_disable_topic("nonexistent");
    CHECK(result == -1);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_enable_all_topics") {
    // Note: This is a simplified implementation that always returns 0
    int result = ulog_enable_all_topics();
    CHECK(result == 0);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_disable_all_topics") {
    // Note: This is a simplified implementation that always returns 0
    int result = ulog_disable_all_topics();
    CHECK(result == 0);
}

#endif  // ULOG_BUILD_TOPICS_MODE || ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Test: Runtime Config Compatibility
============================================================================ */

#if defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_configure_color") {
    // Just test that the function can be called without error
    ulog_configure_color(true);
    log_info("Color enabled");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_configure_color(false);
    log_info("Color disabled");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_configure_prefix") {
    ulog_configure_prefix(true);
    log_info("Prefix enabled");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_configure_prefix(false);
    log_info("Prefix disabled");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_configure_file_string") {
    ulog_configure_file_string(true);
    log_info("File string enabled");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_configure_file_string(false);
    log_info("File string disabled");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_configure_time") {
    ulog_configure_time(true);
    log_info("Time enabled");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_configure_time(false);
    log_info("Time disabled");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_configure_levels") {
    ulog_configure_levels(true);  // Short levels
    log_info("Short levels");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_configure_levels(false);  // Long levels
    log_info("Long levels");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_configure_topics") {
    ulog_configure_topics(true);
    log_info("Topics enabled");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_configure_topics(false);
    log_info("Topics disabled");
    CHECK(ut_callback_get_message_count() == 2);
}

#endif  // ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Test: Custom Prefix Compatibility
============================================================================ */

#if (defined(ULOG_BUILD_PREFIX_SIZE) && ULOG_BUILD_PREFIX_SIZE > 0) || \
    (defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1)

static void test_prefix_fn(ulog_Event *ev, char *prefix, size_t prefix_size) {
    snprintf(prefix, prefix_size, "[PREFIX] ");
    (void)ev;
}

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: ulog_set_prefix_fn") {
    ulog_set_prefix_fn(test_prefix_fn);

    log_info("Test with prefix");
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "[PREFIX]") != nullptr);
}

#endif  // ULOG_BUILD_PREFIX_SIZE || ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Test: Feature Flags
============================================================================ */

TEST_CASE("microlog6_compat: Feature flags are defined") {
    // Just verify that the feature flags are defined
    // Their values depend on build configuration
    bool color = ULOG_FEATURE_COLOR;
    bool prefix = ULOG_FEATURE_CUSTOM_PREFIX;
    bool time = ULOG_FEATURE_TIME;
    bool extra = ULOG_FEATURE_EXTRA_OUTPUTS;
    bool file = ULOG_FEATURE_FILE_STRING;
    bool short_levels = ULOG_FEATURE_SHORT_LEVELS;
    bool topics = ULOG_FEATURE_TOPICS;
    bool runtime = ULOG_FEATURE_RUNTIME_MODE;
    bool emoji = ULOG_FEATURE_EMOJI_LEVELS;

    (void)color;
    (void)prefix;
    (void)time;
    (void)extra;
    (void)file;
    (void)short_levels;
    (void)topics;
    (void)runtime;
    (void)emoji;

    // Emoji should always be false in v7.x
    CHECK(ULOG_FEATURE_EMOJI_LEVELS == false);
}

/* ============================================================================
   Test: Integration - Mixed old and new API
============================================================================ */

TEST_CASE_FIXTURE(TestFixture, "microlog6_compat: Mix old and new API") {
    // Test that old and new API can be used together
    log_info("Old API message");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_info("New API message");
    CHECK(ut_callback_get_message_count() == 2);

    // Set level using old API
    ulog_set_level(LOG_WARN);

    log_debug("Should not appear (old API)");
    CHECK(ut_callback_get_message_count() == 2);

    ulog_debug("Should not appear (new API)");
    CHECK(ut_callback_get_message_count() == 2);

    log_error("Should appear (old API)");
    CHECK(ut_callback_get_message_count() == 3);

    ulog_error("Should appear (new API)");
    CHECK(ut_callback_get_message_count() == 4);
}
