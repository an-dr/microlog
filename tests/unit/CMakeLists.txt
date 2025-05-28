# CMakeLists.txt for microlog unit tests

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/tmp CACHE PATH "FetchContent base dir")

include(FetchContent)

# Fetch Catch2 v3
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2)

FetchContent_MakeAvailable(Catch2)

# Common sources and include directories
set(MICROLOG_SRC ../../src/ulog.c)
set(MICROLOG_INCLUDE_DIR ../../include)

# --- Core Tests ---
add_executable(test_core)
target_sources(test_core PRIVATE ${MICROLOG_SRC} ut_callback.c
                                 test_core.cpp)
target_include_directories(test_core PRIVATE ${MICROLOG_INCLUDE_DIR})
target_link_libraries(test_core PRIVATE Catch2::Catch2WithMain)
target_compile_definitions(test_core PRIVATE "ULOG_EXTRA_OUTPUTS=1")
# add_test(NAME CoreTests COMMAND test_core)

include(CTest)
include(Catch)
catch_discover_tests(test_core)
