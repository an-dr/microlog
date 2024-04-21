# *************************************************************************
#
# Copyright (c) 2024 Andrei Gramakov. All rights reserved.
#
# This file is licensed under the terms of the MIT license.  
# For a copy, see: https://opensource.org/licenses/MIT
#
# site:    https://agramakov.me
# e-mail:  mail@agramakov.me
#
# Andrei's Build CMake subsystem or abcmake is a CMake module to work 
# with C/C++ project of a predefined standard structure in order to 
# simplify the build process.
# 
# Source Code: https://github.com/an-dr/abcmake
# *************************************************************************

set(ABCMAKE_VERSION_MAJOR 5)
set(ABCMAKE_VERSION_MINOR 1)
set(ABCMAKE_VERSION_PATCH 1)
set(ABCMAKE_VERSION "${ABCMAKE_VERSION_MAJOR}.${ABCMAKE_VERSION_MINOR}.${ABCMAKE_VERSION_PATCH}")

# Configure CMake
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CMakeParseArguments)

# *************************************************************************
# Private functions
# *************************************************************************

# Add subdirectory to the project only if not added
function(_add_subdirectory PATH)

    # ABCMAKE_ADDED_PROJECTS is an interface, it may break compatibility if changed!
    get_property(projects GLOBAL PROPERTY ABCMAKE_ADDED_PROJECTS)
    
    # Resolve relative path
    get_filename_component(PATH "${PATH}" ABSOLUTE)
    
    if (NOT PATH IN_LIST projects)
        # Add PATH to the global list
        set_property(GLOBAL APPEND PROPERTY ABCMAKE_ADDED_PROJECTS ${PATH})
        
        # Use the last directory name for a binary directory name 
        get_filename_component(last_dir "${PATH}" NAME)
        add_subdirectory(${PATH} abc_${last_dir})
    endif()
    
endfunction()


function(_abc_AddProject PATH OUT_ABCMAKE_VER)
    if (EXISTS ${PATH}/CMakeLists.txt)
        message(DEBUG "Adding project ${PATH}")
        _add_subdirectory(${PATH})
        
        get_directory_property(version DIRECTORY ${PATH} ABCMAKE_VERSION)
        set(${OUT_ABCMAKE_VER} ${version} PARENT_SCOPE)
        if (NOT version)
            message (STATUS "  🔶 ${PATH} is not an ABCMAKE project. Link it manually.")
        endif()
        
    else()
        message (STATUS "  ❌ ${PATH} is not a CMake project")
    endif()
endfunction()


# Add all projects from the components subdirectory
# @param TARGETNAME - name of the target to add components
function(_abc_AddComponents TARGETNAME)
    # List of possible subprojects
    file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/components ${CMAKE_CURRENT_SOURCE_DIR}/components/*)
    
    # Link all subprojects to the ${TARGETNAME}
    foreach(child ${children})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/components/${child})
            target_link_component(${TARGETNAME} ${CMAKE_CURRENT_SOURCE_DIR}/components/${child})
        endif()
    endforeach()
    
endfunction()


# Install the target near the build directory
# @param TARGETNAME - name of the target to install
# @param DESTINATION - path to the destination directory inside the install dir
function(_target_install_near_build TARGETNAME DESTINATION)
    # install directory
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../install"
         CACHE PATH "default install path" FORCE)
    install(TARGETS ${TARGETNAME} DESTINATION ${DESTINATION})
endfunction()


# Add to the project all files from ./src, ./include, ./lib
# @param TARGETNAME - name of the target to initialize
# @param INCLUDE_DIR - path to the include directory
# @param SOURCE_DIR - path to the source directory
function(_target_init_abcmake TARGETNAME INCLUDE_DIR SOURCE_DIR)

    get_directory_property(hasParent PARENT_DIRECTORY)
    # if no parent, print the name of the target
    if (NOT hasParent)
        message(STATUS "🔤 ${TARGETNAME}")
    endif ()
    
    # Report version
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY 
                 ABCMAKE_VERSION ${ABCMAKE_VERSION})
                 
    # Add target to the target list
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} APPEND PROPERTY
                 ABCMAKE_TARGETS ${TARGETNAME})
        
    target_sources_directory(${TARGETNAME} ${SOURCE_DIR})
    target_include_directories(${TARGETNAME} PUBLIC ${INCLUDE_DIR})
    _abc_AddComponents(${TARGETNAME})

endfunction()

# *************************************************************************
# Public functions
# *************************************************************************

# Add all source files from the specified directory to the target
# @param TARGETNAME - name of the target to add sources
function(target_sources_directory TARGETNAME SOURCE_DIR)
    file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")
    message( DEBUG "${TARGETNAME} sources: ${SOURCES}")
    target_sources(${TARGETNAME} PRIVATE ${SOURCES})
endfunction()



# Link the component to the target
# @param TARGETNAME - name of the target for linking
# @param COMPONENTPATH - path to the component to link
function (target_link_component TARGETNAME COMPONENTPATH)
    _abc_AddProject(${COMPONENTPATH} ver)
    if (ver)
        get_directory_property(to_link DIRECTORY ${COMPONENTPATH} ABCMAKE_TARGETS)
        message (STATUS "  ✅ Linking ${to_link} to ${TARGETNAME}")
        target_link_libraries(${TARGETNAME} PRIVATE ${to_link})
    endif()
endfunction()

# Add an executable component to the project
# @param TARGETNAME - name of the target to add the component
# @param INCLUDE_DIR - path to the include directory
# @param SOURCE_DIR - path to the source directory
function(add_main_component TARGETNAME)
    set(flags)
    set(args)
    set(listArgs INCLUDE_DIR SOURCE_DIR)
    cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})
    
    if (NOT arg_SOURCE_DIR)
        set(arg_SOURCE_DIR "src")
    endif()
    
    if (NOT arg_INCLUDE_DIR)
        set(arg_INCLUDE_DIR "include")
    endif()
    
    add_executable(${TARGETNAME})
    _target_init_abcmake(${TARGETNAME} ${arg_INCLUDE_DIR} ${arg_SOURCE_DIR})
    _target_install_near_build(${TARGETNAME} ".")
endfunction()

# Add a shared or static library component to the project
# @param TARGETNAME - name of the target to add the component
# @param INCLUDE_DIR - path to the include directory
# @param SOURCE_DIR - path to the source directory
# @param SHARED - if set to TRUE, the library will be shared
function(add_component TARGETNAME)
    set(flags SHARED)
    set(args)
    set(listArgs INCLUDE_DIR SOURCE_DIR)
    cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})

    if (NOT arg_SOURCE_DIR)
        set(arg_SOURCE_DIR "src")
    endif()

    if (NOT arg_INCLUDE_DIR)
        set(arg_INCLUDE_DIR "include")
    endif()

    if (arg_SHARED)
        add_library(${TARGETNAME} SHARED)
    else()
        add_library(${TARGETNAME} STATIC)
    endif()
    
    _target_init_abcmake(${TARGETNAME} ${arg_INCLUDE_DIR} ${arg_SOURCE_DIR})
    _target_install_near_build(${TARGETNAME} "lib")
endfunction()
