# *************************************************************************
#
# Copyright (c) 2025 Andrei Gramakov. All rights reserved.
#
# This file is licensed under the terms of the MIT license. For a copy, see:
# https://opensource.org/licenses/MIT
#
# site:    https://agramakov.me e-mail:  mail@agramakov.me
#
# *************************************************************************
cmake_minimum_required(VERSION 3.15.0)

# Read version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version" VERSION_CONTENT)
string(STRIP "${VERSION_CONTENT}" PROJECT_VERSION)

project(microlog VERSION ${PROJECT_VERSION})

# ----------------------------------------------------------------------------
# Building
# ----------------------------------------------------------------------------

file(GLOB_RECURSE MICROLOG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
set(AMALGAMATED_C_FILE "${CMAKE_CURRENT_BINARY_DIR}/microlog_amalgamated.c")

add_custom_command(
    OUTPUT ${AMALGAMATED_C_FILE}
    COMMAND ${CMAKE_COMMAND} -E cat ${MICROLOG_SOURCES} > ${AMALGAMATED_C_FILE}
    DEPENDS ${MICROLOG_SOURCES}
    COMMENT "Amalgamating C sources: ${MICROLOG_SOURCES} into ${AMALGAMATED_C_FILE}"
    VERBATIM
)

# This target ensures the custom command is triggered as part of the default build.
add_custom_target(MicrologAmalgamate ALL DEPENDS ${AMALGAMATED_C_FILE})

add_library(${PROJECT_NAME} INTERFACE)
target_sources(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${AMALGAMATED_C_FILE}>
            $<INSTALL_INTERFACE:src/ulog.c> # installed version will assume
                                            # relative to install prefix, named ulog.c
)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include> # while
                                                                 # building
            $<INSTALL_INTERFACE:include>) # when installed

include(CTest)
enable_testing()

# ----------------------------------------------------------------------------
# Testing
# ----------------------------------------------------------------------------

if(ULOG_BUILD_TESTS)
  message(STATUS "Building tests")
  add_subdirectory(tests/unit)
else()
  message(STATUS "Skipping tests")
endif()

# ----------------------------------------------------------------------------
# Installing
# ----------------------------------------------------------------------------

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib)

# Headers (they are sources, not artifacts so copy them explicitly)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# Install the amalgamated source file, renaming it to ulog.c
install(
    FILES ${AMALGAMATED_C_FILE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/src
    RENAME ulog.c
)

# ----------------------------------------------------------------------------
# Packing
# ----------------------------------------------------------------------------
include(CMakePackageConfigHelpers)

# We have a custom Targets file below. Original is generated like this: install(
# EXPORT ${PROJECT_NAME}Targets NAMESPACE ${PROJECT_NAME}:: DESTINATION
# ${CMAKE_INSTALL_PREFIX})

install(FILES ${CMAKE_CURRENT_LIST_DIR}/cmake/micrologTargets.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX})

# Make a config file using targets based on the template
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX})

# Make a version file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion)

# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
# DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        ${CMAKE_CURRENT_LIST_DIR}/LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX})
