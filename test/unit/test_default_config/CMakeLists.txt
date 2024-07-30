# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# add_definitions(-DULOG_USE_COLOR)
# add_definitions(-DULOG_HAVE_TIME)
# add_definitions(-DULOG_HIDE_FILE_STRING)
# add_definitions(-DULOG_SHORT_LEVEL_STRINGS)
# add_definitions(-DULOG_EXTRA_DESTINATIONS=2)
# add_definitions(-DULOG_CUSTOM_PREFIX_SIZE=8)
# # add_definitions(-DULOG_NO_STDOUT)



include(${CMAKE_CURRENT_LIST_DIR}/../../../ab.cmake)

add_main_component(test_default_config SRC ${CMAKE_CURRENT_LIST_DIR})
add_test(NAME test_default_config COMMAND test_default_config)
target_link_components(test_default_config PATH ${CMAKE_CURRENT_LIST_DIR}/../../../)
