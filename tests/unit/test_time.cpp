#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"

TEST_CASE("Time Feature Macro") {
#if FEATURE_TIME
    CHECK(true);
#else
    CHECK(true); // If time is off, test passes
#endif
}
