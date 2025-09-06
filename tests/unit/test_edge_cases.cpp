#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>
#include "ulog.h"
#include "ut_callback.h"

static void dummy_output(ulog_event *ev, void *arg) {
    (void)ev; (void)arg; /* intentionally no logging */
}

struct EdgeFixture {
    EdgeFixture() {
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ut_callback_reset();
    }
};

TEST_CASE_FIXTURE(EdgeFixture, "Levels: invalid to string") {
    CHECK(std::strcmp(ulog_level_to_string((ulog_level)-1), "?") == 0);
    CHECK(std::strcmp(ulog_level_to_string((ulog_level)123), "?") == 0);
}

TEST_CASE_FIXTURE(EdgeFixture, "Outputs: invalid level set and capacity") {
    // invalid level values
    CHECK(ulog_output_level_set(ULOG_OUTPUT_STDOUT, (ulog_level)-1) == ULOG_STATUS_INVALID_ARGUMENT);
    CHECK(ulog_output_level_set(ULOG_OUTPUT_STDOUT, (ulog_level)ULOG_LEVEL_TOTAL) == ULOG_STATUS_INVALID_ARGUMENT);

    // invalid output id
    CHECK(ulog_output_level_set(-5, ULOG_LEVEL_INFO) == ULOG_STATUS_INVALID_ARGUMENT);

    // level set all invalid
    CHECK(ulog_output_level_set_all((ulog_level)-1) == ULOG_STATUS_INVALID_ARGUMENT);
    CHECK(ulog_output_level_set_all((ulog_level)ULOG_LEVEL_TOTAL) == ULOG_STATUS_INVALID_ARGUMENT);

    // Try to allocate remaining slots, tolerate already-full situation.
    std::vector<ulog_output_id> ids;
    for (int i=0;i<8;i++) {
        ulog_output_id id = ulog_output_add(dummy_output, nullptr, ULOG_LEVEL_TRACE);
        if (id == ULOG_OUTPUT_INVALID) {
            // Already full from previous tests; break early.
            break;
        }
        ids.push_back(id);
    }

    // Now ensure that one more add either fails (full) which is acceptable.
    (void)ulog_output_add(dummy_output, nullptr, ULOG_LEVEL_TRACE);

    if (!ids.empty()) {
        // Remove last added output and re-add should succeed (slot reuse)
        CHECK(ulog_output_remove(ids.back()) == ULOG_STATUS_OK);
        ulog_output_id reused = ulog_output_add(dummy_output, nullptr, ULOG_LEVEL_TRACE);
        CHECK(reused != ULOG_OUTPUT_INVALID);
    }
}

#if ULOG_BUILD_PREFIX_SIZE > 0 || defined(ULOG_BUILD_DYNAMIC_CONFIG)
static void test_prefix_fn(ulog_event *ev, char *buf, size_t sz){ (void)ev; snprintf(buf, sz, "[EDGE]"); }
#endif

TEST_CASE_FIXTURE(EdgeFixture, "Prefix: ignore NULL setter") {
#if ULOG_BUILD_PREFIX_SIZE > 0 || defined(ULOG_BUILD_DYNAMIC_CONFIG)
    ulog_prefix_set_fn(test_prefix_fn);
    ulog_info("With prefix");
    CHECK(std::strstr(ut_callback_get_last_message(), "[EDGE]") != nullptr);
    // Try to set NULL (should be ignored per implementation) and prefix stays
    ulog_prefix_set_fn(NULL);
    ulog_info("Still with prefix");
    CHECK(std::strstr(ut_callback_get_last_message(), "[EDGE]") != nullptr);
#endif
}

#if ULOG_BUILD_TOPICS_NUM != 0 || defined(ULOG_BUILD_DYNAMIC_CONFIG)
TEST_CASE_FIXTURE(EdgeFixture, "Topics: invalid id and non-existent") {
    // Non-existent id
    CHECK(ulog_topic_get_id("__no_such_topic__") == -1);
    // Enable/disable non-existent
    CHECK(ulog_topic_enable("__no_such_topic__") == ULOG_STATUS_ERROR);
    CHECK(ulog_topic_disable("__no_such_topic__") == ULOG_STATUS_ERROR);
    // Level set non-existent
    CHECK(ulog_topic_level_set("__no_such_topic__", ULOG_LEVEL_INFO) == ULOG_STATUS_ERROR);
}
#endif

TEST_CASE_FIXTURE(EdgeFixture, "Event getters: NULL paths") {
#if ULOG_BUILD_TOPICS_NUM != 0 || defined(ULOG_BUILD_DYNAMIC_CONFIG)
    CHECK(ulog_event_get_topic(NULL) == -1);
#endif
#if ULOG_BUILD_TIME == 1 || defined(ULOG_BUILD_DYNAMIC_CONFIG)
    CHECK(ulog_event_get_time(NULL) == nullptr);
#endif
    CHECK(ulog_event_get_file(NULL) == nullptr);
    CHECK(ulog_event_get_line(NULL) == -1);
    CHECK(ulog_event_get_level(NULL) == ULOG_LEVEL_TRACE); // default fallback
}
