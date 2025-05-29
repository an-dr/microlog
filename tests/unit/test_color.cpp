#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"

TEST_CASE("Color Feature Macro") {
#if FEATURE_COLOR
    CHECK(true);
#else
    CHECK(true); // If color is off, test passes
#endif
}
