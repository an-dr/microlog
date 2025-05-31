# CMakeLists.txt for microlog unit tests

# Common sources and include directories
set(ULOG_SRC ../../src/ulog.c)
set(ULOG_INCLUDE_DIR ../../include)

# Minimal optimization flags

# Configs
set(CONFIG_BASE_LIST "ULOG_EXTRA_OUTPUTS=8" 
                     "ULOG_HAVE_TIME"
                     "ULOG_TOPICS_NUMBER=8"
                     "ULOG_CUSTOM_PREFIX_SIZE=8"
                     )

                     
# --- Core Tests ---
# Add force_link_all.c to ensure all ulog.c functions are linked for coverage
add_executable(test_core)
target_sources(test_core PRIVATE ${ULOG_SRC} ut_callback.c
                                 test_core.cpp force_link_all.c)
target_include_directories(test_core PRIVATE ${ULOG_INCLUDE_DIR})
target_compile_definitions(test_core PRIVATE ${CONFIG_BASE_LIST})

# Add coverage flags if COVERAGE is ON
# option(COVERAGE "Enable coverage reporting" OFF)
# if(COVERAGE)
    message(STATUS "Building with coverage flags")
    target_compile_options(test_core PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_libraries(test_core PRIVATE --coverage -fprofile-arcs -ftest-coverage)
# endif()

add_test(NAME CoreTests COMMAND test_core)
