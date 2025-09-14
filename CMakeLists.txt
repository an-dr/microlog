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

# Read version (raw file contains semantic version optionally with metadata)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version" VERSION_CONTENT)
string(STRIP "${VERSION_CONTENT}" PROJECT_VERSION_RAW)
string(REGEX REPLACE "^([0-9]+\.[0-9]+\.[0-9]+).*$" "\\1" PROJECT_VERSION "${PROJECT_VERSION_RAW}")
project(microlog VERSION ${PROJECT_VERSION})

# Expose version for substitution
set(ULOG_VERSION "${PROJECT_VERSION_RAW}")

# We keep original sources in-place without .in duplicates. For install we
# generate configured copies with the banner token replaced. This avoids
# maintaining parallel template files while still delivering substituted
# content to consumers.

set(MICROLOG_FILES_CONFIGURE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ulog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ulog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/extensions/ulog_syslog.h
)

set(MICROLOG_CONFIGURED_DIR ${CMAKE_CURRENT_BINARY_DIR}/configured)
file(MAKE_DIRECTORY ${MICROLOG_CONFIGURED_DIR}/include)
file(MAKE_DIRECTORY ${MICROLOG_CONFIGURED_DIR}/src)
file(MAKE_DIRECTORY ${MICROLOG_CONFIGURED_DIR}/extensions)

foreach(f ${MICROLOG_FILES_CONFIGURE})
  get_filename_component(fname ${f} NAME)
  # Decide subdir based on original path
  if(f MATCHES "/include/")
    set(out "${MICROLOG_CONFIGURED_DIR}/include/${fname}")
  elseif(f MATCHES "/src/")
    set(out "${MICROLOG_CONFIGURED_DIR}/src/${fname}")
  elseif(f MATCHES "/extensions/")
    set(out "${MICROLOG_CONFIGURED_DIR}/extensions/${fname}")
  else()
    set(out "${MICROLOG_CONFIGURED_DIR}/${fname}")
  endif()
  configure_file(${f} ${out} @ONLY NEWLINE_STYLE UNIX)
endforeach()

# Provide an interface include dir to configured headers *after* install while
# using original headers for build to avoid recompilation on each clean.
set(MICROLOG_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(MICROLOG_INSTALL_INCLUDE_DIR include)

# ----------------------------------------------------------------------------
# Building
# ----------------------------------------------------------------------------

add_library(${PROJECT_NAME} INTERFACE)
target_sources(microlog
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/ulog.c>
    $<INSTALL_INTERFACE:src/ulog.c>
)
target_include_directories(${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${MICROLOG_BUILD_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

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
install(DIRECTORY ${MICROLOG_CONFIGURED_DIR}/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(FILES ${MICROLOG_CONFIGURED_DIR}/src/ulog.c DESTINATION ${CMAKE_INSTALL_PREFIX}/src)
install(FILES ${MICROLOG_CONFIGURED_DIR}/extensions/ulog_syslog.h DESTINATION ${CMAKE_INSTALL_PREFIX}/extensions)

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
