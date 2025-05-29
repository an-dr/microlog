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
