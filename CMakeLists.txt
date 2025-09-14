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

# ----------------------------------------------------------------------------
# Project
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.15.0)

# Read version (raw file contains semantic version optionally with metadata)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version" VERSION_CONTENT)
string(STRIP "${VERSION_CONTENT}" PROJECT_VERSION_RAW)
string(REGEX REPLACE "^([0-9]+\.[0-9]+\.[0-9]+).*$" "\\1" PROJECT_VERSION
                     "${PROJECT_VERSION_RAW}")

# Declare the project
project(microlog VERSION ${PROJECT_VERSION})

# ----------------------------------------------------------------------------
# Version substitution
# ----------------------------------------------------------------------------
# Expose version for substitution
set(ULOG_VERSION "${PROJECT_VERSION_RAW}")

# Configure selected sources/headers preserving relative paths
set(ULOG_CONFIGURED_DIR ${CMAKE_CURRENT_BINARY_DIR}/configured)

# Treat source files as templates
set(ULOG_FILES_CONFIGURE src/ulog.c include/ulog.h)

# Process each template
foreach(input_file ${ULOG_FILES_CONFIGURE})

  # Create subdirs as needed
  get_filename_component(dir ${input_file} DIRECTORY)
  if(dir)
    file(MAKE_DIRECTORY ${ULOG_CONFIGURED_DIR}/${dir})
  endif()

  # Configure file
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${input_file}
                 ${ULOG_CONFIGURED_DIR}/${input_file} @ONLY NEWLINE_STYLE UNIX)
endforeach()

# ----------------------------------------------------------------------------
# Building
# ----------------------------------------------------------------------------
set(ULOG_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(ULOG_BUILD_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(ULOG_INSTALL_INCLUDE_DIR .)
set(ULOG_INSTALL_SRC_DIR src)

add_library(${PROJECT_NAME} INTERFACE)
# Provide namespaced alias when consumed via add_subdirectory
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(
  microlog
  INTERFACE $<BUILD_INTERFACE:${ULOG_BUILD_SRC_DIR}/ulog.c>
            $<INSTALL_INTERFACE:${ULOG_INSTALL_SRC_DIR}/ulog.c> # installed
                                                                # version
            # will assume relative to install prefix
)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${ULOG_BUILD_INCLUDE_DIR}> # while building
            $<INSTALL_INTERFACE:${ULOG_INSTALL_INCLUDE_DIR}> # when
            # installed
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
set(ULOG_INSTALL_INCLUDE_IN ${ULOG_CONFIGURED_DIR}/include)
set(ULOG_INSTALL_SRC_IN ${ULOG_CONFIGURED_DIR}/src/ulog.c)
set(ULOG_INSTALL_INCLUDE_DST
    ${CMAKE_INSTALL_PREFIX}/${ULOG_INSTALL_INCLUDE_DIR})
set(ULOG_INSTALL_SRC_DST ${CMAKE_INSTALL_PREFIX}/${ULOG_INSTALL_SRC_DIR})

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib)

# Headers (they are sources, not artifacts so copy them explicitly)
install(DIRECTORY ${ULOG_INSTALL_INCLUDE_IN}
        DESTINATION ${ULOG_INSTALL_INCLUDE_DST})
install(FILES ${ULOG_INSTALL_SRC_IN} DESTINATION ${ULOG_INSTALL_SRC_DST})

# ----------------------------------------------------------------------------
# Packing
# ----------------------------------------------------------------------------
set(ULOG_CONFIG_FILE_IN ${CMAKE_CURRENT_LIST_DIR}/cmake/micrologConfig.cmake.in)
set(ULOG_CONFIG_FILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/micrologConfig.cmake)
set(ULOG_TARGETS_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/micrologTargets.cmake)
set(ULOG_VERSION_FILE_OUT
    ${CMAKE_CURRENT_BINARY_DIR}/micrologConfigVersion.cmake)

include(CMakePackageConfigHelpers)

# We have a custom Targets file below. Original is generated like this: install(
# EXPORT ${PROJECT_NAME}Targets NAMESPACE ${PROJECT_NAME}:: DESTINATION
# ${CMAKE_INSTALL_PREFIX})
install(FILES ${ULOG_TARGETS_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX})

# Make a config file using targets based on the template
configure_package_config_file(${ULOG_CONFIG_FILE_IN} ${ULOG_CONFIG_FILE_OUT}
                              INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX})

# Make a version file
write_basic_package_version_file(
  ${ULOG_VERSION_FILE_OUT}
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES ${ULOG_CONFIG_FILE_OUT} ${ULOG_VERSION_FILE_OUT}
              ${CMAKE_CURRENT_LIST_DIR}/LICENSE
        DESTINATION ${CMAKE_INSTALL_PREFIX})
