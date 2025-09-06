#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"

// This test target is built with all optional features disabled but warn stubs enabled.
// We just invoke the APIs to ensure they return safe defaults and do not crash.

TEST_CASE("WarnNotEnabled: disabled feature stubs are safe") {
    // Dynamic config APIs should just warn internally and not crash.
    ulog_color_config(true);
    ulog_prefix_config(true);
    ulog_source_location_config(true);
    ulog_time_config(true);
    ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_SHORT);
    ulog_topic_config(true);

    // Output APIs (extra outputs disabled)
    CHECK(ulog_output_add(NULL, NULL, ULOG_LEVEL_INFO) == ULOG_OUTPUT_INVALID);
    CHECK(ulog_output_add_file(NULL, ULOG_LEVEL_INFO) == ULOG_OUTPUT_INVALID);
    CHECK(ulog_output_remove(1) == ULOG_STATUS_ERROR);

    // Topic APIs (topics disabled)
    CHECK(ulog_topic_add("t0", ULOG_OUTPUT_STDOUT, true) == ULOG_TOPIC_ID_INVALID);
    CHECK(ulog_topic_get_id("t0") == ULOG_TOPIC_ID_INVALID);
    CHECK(ulog_topic_enable("t0") == ULOG_STATUS_ERROR);
    CHECK(ulog_topic_disable("t0") == ULOG_STATUS_ERROR);
    CHECK(ulog_topic_enable_all() == ULOG_STATUS_ERROR);
    CHECK(ulog_topic_disable_all() == ULOG_STATUS_ERROR);

    // Logging with disabled features should still succeed (no crash)
    ulog_info("Hello disabled world");
}
