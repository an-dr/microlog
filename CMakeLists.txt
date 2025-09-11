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
string(STRIP "${VERSION_CONTENT}" PROJECT_VERSION_RAW)
string(REGEX REPLACE "^([0-9]+\\.[0-9]+\\.[0-9]+).*$" "\\1" PROJECT_VERSION "${PROJECT_VERSION_RAW}")

project(microlog VERSION ${PROJECT_VERSION})

# ----------------------------------------------------------------------------
# Building
# ----------------------------------------------------------------------------

option(MICROLOG_EXT_SYSLOG "Build syslog levels extension (ulog_syslog)" ON)

add_library(${PROJECT_NAME} INTERFACE)

set(MICROLOG_PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ulog.h)

set(MICROLOG_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ulog.c)

if(MICROLOG_EXT_SYSLOG)
  list(APPEND MICROLOG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/extensions/ulog_syslog.c)
endif()

target_sources(
  microlog
  INTERFACE $<BUILD_INTERFACE:${MICROLOG_SOURCES}>
            $<INSTALL_INTERFACE:src/ulog.c> # keep core source; extensions installed separately
)
target_include_directories(${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/extensions>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:extensions>)

# ----------------------------------------------------------------------------
# Testing
# ----------------------------------------------------------------------------

if(ULOG_BUILD_TESTS)
  include(CTest)
  enable_testing()
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
install(FILES src/ulog.c DESTINATION ${CMAKE_INSTALL_PREFIX}/src)
install(FILES extensions/ulog_syslog.h DESTINATION ${CMAKE_INSTALL_PREFIX}/extensions)
if(MICROLOG_EXT_SYSLOG)
  install(FILES extensions/ulog_syslog.c DESTINATION ${CMAKE_INSTALL_PREFIX}/extensions)
endif()

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
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
# DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        ${CMAKE_CURRENT_LIST_DIR}/LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX})
