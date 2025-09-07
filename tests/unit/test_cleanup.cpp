// Test for ulog_cleanup
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include "ulog.h"
#include "ut_callback.h"

static void test_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PFX]");
}

TEST_CASE("Cleanup: outputs, topics, prefix reset") {
    // Ensure trace level so all messages pass
    ulog_output_level_set_all(ULOG_LEVEL_TRACE);

    // Register callback output
    ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
    ut_callback_reset();

    // 1) Basic log before anything else
    ulog_info("Pre-cleanup baseline");
    CHECK(ut_callback_get_message_count() == 1);

    // 2) Add prefix and topic and log
    ulog_prefix_set_fn(test_prefix);
    ulog_topic_add("cleanup_topic", ULOG_OUTPUT_ALL, true);
    ulog_t_info("cleanup_topic", "With prefix and topic");
    CHECK(ut_callback_get_message_count() == 2);
    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);
    CHECK(strstr(msg, "[PFX]") != nullptr);
    CHECK(strstr(msg, "[cleanup_topic]") != nullptr);

    // 3) Cleanup
    REQUIRE(ulog_cleanup() == ULOG_STATUS_OK);

    // Logging now should NOT increase counter (callback removed)
    ulog_info("After cleanup should not be captured");
    CHECK(ut_callback_get_message_count() == 2);

    // 4) Re-add callback only (no prefix function re-registered yet)
    ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
    ulog_info("Post cleanup no prefix");
    CHECK(ut_callback_get_message_count() == 3);
    msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);
    CHECK(strstr(msg, "[PFX]") == nullptr); // prefix function cleared

    // 5) Recreate topic and ensure it works again (ID may restart)
    ulog_topic_add("cleanup_topic", ULOG_OUTPUT_ALL, true);
    ulog_t_info("cleanup_topic", "Recreated topic");
    CHECK(ut_callback_get_message_count() == 4);
    msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);
    CHECK(strstr(msg, "[cleanup_topic]") != nullptr);
}
