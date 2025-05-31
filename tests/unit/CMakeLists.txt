# CMakeLists.txt for microlog unit tests

# Common sources and include directories
set(MICROLOG_SRC ../../src/ulog.c)
set(MICROLOG_INCLUDE_DIR ../../include)

# Configs
set(CONFIG_BASE_LIST "ULOG_EXTRA_OUTPUTS=8" 
                     "ULOG_HAVE_TIME"
                     "ULOG_TOPICS_NUMBER=8"
                     "ULOG_CUSTOM_PREFIX_SIZE=8"
                     )

# --- Core Tests ---
add_executable(test_core)
target_sources(test_core PRIVATE ${MICROLOG_SRC} ut_callback.c
                                 test_core.cpp)
target_include_directories(test_core PRIVATE ${MICROLOG_INCLUDE_DIR})
target_compile_definitions(test_core PRIVATE ${CONFIG_BASE_LIST})

# Add coverage flags if COVERAGE is ON
# option(COVERAGE "Enable coverage reporting" OFF)
# if(COVERAGE)
    message(STATUS "Building with coverage flags")
    target_compile_options(test_core PRIVATE --coverage)
    target_link_libraries(test_core PRIVATE --coverage)
# endif()

add_test(NAME CoreTests COMMAND test_core)
