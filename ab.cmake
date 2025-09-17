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

set(ABCMAKE_VERSION_MAJOR 6)
set(ABCMAKE_VERSION_MINOR 1)
set(ABCMAKE_VERSION_PATCH 0)
set(ABCMAKE_VERSION "${ABCMAKE_VERSION_MAJOR}.${ABCMAKE_VERSION_MINOR}.${ABCMAKE_VERSION_PATCH}")


# Configure CMake
set(ABCMAKE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# ----------------------------------------------------------------------------
# Internal CMake modules
# ----------------------------------------------------------------------------

# ==============================================================================
# _abcmake_log.cmake.cmake =====================================================

set(__ABCMAKE_INDENTATION "  ")

if ($ENV{ABCMAKE_EMOJI})
    set(__ABCMAKE_COMPONENT "🔤")
    set(__ABCMAKE_OK "✅")
    set(__ABCMAKE_ERROR "❌")
    set(__ABCMAKE_WARNING "🔶")
    set(__ABCMAKE_NOTE "⬜")
else()
    set(__ABCMAKE_COMPONENT "[ABCMAKE]")
    set(__ABCMAKE_OK        "[DONE]")
    set(__ABCMAKE_ERROR     "[ERROR]")
    set(__ABCMAKE_WARNING   "[WARNING]")
    set(__ABCMAKE_NOTE      "[INFO]")
endif()

# Print a message with indentation
# @param INDENTATION The indentation level. If < 0, the message is not printed
# @param MESSAGE The message to print
function(_abcmake_log INDENTATION MESSAGE)
    if(${INDENTATION} GREATER_EQUAL 0)
        string(REPEAT ${__ABCMAKE_INDENTATION} ${INDENTATION} indentation)
        message(STATUS "${indentation}${MESSAGE}")
    endif()
endfunction()

function(_abcmake_log_ok INDENTATION MESSAGE)
    _abcmake_log(${INDENTATION} "${__ABCMAKE_OK} ${MESSAGE}")
endfunction()

function(_abcmake_log_err INDENTATION MESSAGE)
    _abcmake_log(${INDENTATION} "${__ABCMAKE_ERROR} ${MESSAGE}")
endfunction()

function(_abcmake_log_warn INDENTATION MESSAGE)
    _abcmake_log(${INDENTATION} "${__ABCMAKE_WARNING} ${MESSAGE}")
endfunction()

function(_abcmake_log_note INDENTATION MESSAGE)
    _abcmake_log(${INDENTATION} "${__ABCMAKE_NOTE} ${MESSAGE}")
endfunction()

function(_abcmake_log_header INDENTATION MESSAGE)
    _abcmake_log(${INDENTATION} "${__ABCMAKE_COMPONENT} ${MESSAGE}")
endfunction()


# _abcmake_log.cmake.cmake =====================================================
# ==============================================================================

# ==============================================================================
# abcmake_property.cmake =======================================================

# A change in any of these variables will cause a breaking change in the API!

set(ABCMAKE_PROPERTY_PREFIX "ABCMAKE_")

# Global properties:
set(ABCMAKE_PROP_ADDED_PROJECTS "ADDED_PROJECTS") # The list of projects that have been added to the solution
set(ABCMAKE_PROP_COMPONENT_REGISTRY "COPONENT_REGISTRY") # Component registry
set(_ABCMAKE_PROP_COMPONENTS_DIR "COMPONENTS_DIR") # The directory where the components are stored
set(_ABCMAKE_PROP_SRC_DIR "SRC_DIR") # The directory where the source files are stored
set(_ABCMAKE_PROP_INCLUDE_DIR "INCLUDE_DIR") # The directory where the include files are stored
set(_ABCMAKE_PROP_INSTALL_DIR "INSTALL_DIR") # The directory where the project will be installed

# Directory-scope properties
set(ABCMAKE_DIRPROP_VERSION "VERSION") # The abcmake version of the component
set(ABCMAKE_DIRPROP_COMPONENT_NAME "COMPONENT_NAME") # The name of the component (local PROJECT_NAME)
set(ABCMAKE_DIRPROP_TARGETS "TARGETS") # The list of targets built by the component
set(ABCMAKE_DIRPROP_SRC "COMPONENT_SRC") # The list of source directories
set(ABCMAKE_DIRPROP_INCLUDE "COMPONENT_INCLUDE") # The list of include directories

# Default values
set(_ABCMAKE_DEFAULT_COMPONENTS_DIR "components")
set(_ABCMAKE_DEFAULT_SRC_DIR "src")
set(_ABCMAKE_DEFAULT_INCLUDE_DIR "include")
set(_ABCMAKE_DEFAULT_INSTALL_DIR "${CMAKE_BINARY_DIR}/../install")


# Setters
# =======

function(_abcmake_set_prop PROPERTY_NAME PROPERTY_VALUE)
    set_property(GLOBAL PROPERTY 
                 "${ABCMAKE_PROPERTY_PREFIX}${PROPERTY_NAME}" "${PROPERTY_VALUE}")
endfunction()

function(_abcmake_append_prop PROPERTY_NAME PROPERTY_VALUE)
    set_property(GLOBAL APPEND PROPERTY 
                 "${ABCMAKE_PROPERTY_PREFIX}${PROPERTY_NAME}" "${PROPERTY_VALUE}")
endfunction()

function(_abcmake_set_prop_curdir PROPERTY_NAME PROPERTY_VALUE)
    set_directory_properties(PROPERTIES
                             "${ABCMAKE_PROPERTY_PREFIX}${PROPERTY_NAME}" "${PROPERTY_VALUE}")
endfunction()

function(_abcmake_append_prop_curdir PROPERTY_NAME PROPERTY_VALUE)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                 APPEND PROPERTY 
                 "${ABCMAKE_PROPERTY_PREFIX}${PROPERTY_NAME}" "${PROPERTY_VALUE}")
endfunction()


# Getters
# =======

# Get a global property of ABCMAKE (with the ABCMAKE_PROPERTY_PREFIX)
# @param PROPERTY_NAME - The name of the property to get 
# @param OUT_VAR_NAME - The name of the variable to set with the result
# @param FALLBACK - Optional argument, if the property is not found, the value of FALLBACK will be used
function(_abcmake_get_prop PROPERTY_NAME OUT_VAR_NAME)
    # optional argument FALLBACK
    set(flags)
    set(args)
    set(listArgs FALLBACK)
    cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})

    # Getting the property
    get_property(tmp_result GLOBAL PROPERTY 
                 ${ABCMAKE_PROPERTY_PREFIX}${PROPERTY_NAME})
                 
    # If not found, try to use the fallback
    if(NOT tmp_result AND arg_FALLBACK)
        set(tmp_result ${arg_FALLBACK})
    endif()
    
    # Setting the result
    set(${OUT_VAR_NAME} ${tmp_result} PARENT_SCOPE)
endfunction()

function(_abcmake_get_prop_dir DIRECTORY PROPERTY_NAME OUT_VAR_NAME)
    message(DEBUG "[_abcmake_get_prop_dir] Getting property ${ABCMAKE_PROPERTY_PREFIX}${PROPERTY_NAME} from ${DIRECTORY}")
    get_directory_property(tmp_result DIRECTORY ${DIRECTORY}
        ${ABCMAKE_PROPERTY_PREFIX}${PROPERTY_NAME})
    set(${OUT_VAR_NAME} ${tmp_result} PARENT_SCOPE)
endfunction()

# Specific Getters
# =================

function(_abcmake_get_components OUT_VAR_NAME)
    _abcmake_get_prop(${_ABCMAKE_PROP_COMPONENTS_DIR} tmp_result 
                 FALLBACK ${_ABCMAKE_DEFAULT_COMPONENTS_DIR})
    set(${OUT_VAR_NAME} ${tmp_result} PARENT_SCOPE)
endfunction()

function(_abcmake_get_src OUT_VAR_NAME)
    _abcmake_get_prop(${_ABCMAKE_PROP_SRC_DIR} tmp_result  
                 FALLBACK ${_ABCMAKE_DEFAULT_SRC_DIR})
    set(${OUT_VAR_NAME} ${tmp_result} PARENT_SCOPE)
endfunction()

function(_abcmake_get_include OUT_VAR_NAME)
    _abcmake_get_prop(${_ABCMAKE_PROP_INCLUDE_DIR} tmp_result 
                 FALLBACK ${_ABCMAKE_DEFAULT_INCLUDE_DIR})
    set(${OUT_VAR_NAME} ${tmp_result} PARENT_SCOPE)
endfunction()

function(_abcmake_get_install OUT_VAR_NAME)
    _abcmake_get_prop(${_ABCMAKE_PROP_INSTALL_DIR} tmp_result 
                 FALLBACK ${_ABCMAKE_DEFAULT_INSTALL_DIR})
    set(${OUT_VAR_NAME} ${tmp_result} PARENT_SCOPE) 
endfunction()

# abcmake_property.cmake =======================================================
# ==============================================================================


# ==============================================================================
# _abcmake_add_project.cmake ====================================================

# Add subdirectory to the project only if not added
function(_abcmake_add_subdirectory PATH)

    # ABCMAKE_ADDED_PROJECTS is an interface, it may break compatibility if changed!
    _abcmake_get_prop(${ABCMAKE_PROP_ADDED_PROJECTS} projects)
    message(DEBUG "[_abcmake_add_subdirectory] Added projects: ${projects}")
    
    # Resolve relative path
    get_filename_component(PATH "${PATH}" ABSOLUTE)
    
    if (NOT PATH IN_LIST projects)
        # Add PATH to the global list
        _abcmake_append_prop(${ABCMAKE_PROP_ADDED_PROJECTS} ${PATH})
        
        # Use the last directory name for a binary directory name 
        get_filename_component(last_dir "${PATH}" NAME)
        add_subdirectory(${PATH} abc_${last_dir} EXCLUDE_FROM_ALL)
    endif()
    
endfunction()

function(_abcmake_add_project PATH OUT_ABCMAKE_VER)
    if (NOT EXISTS ${PATH})
        _abcmake_log_err(1 "Path \"${PATH}\" does not exist!")
        return()
    endif()
    
    if (NOT EXISTS ${PATH}/CMakeLists.txt)
        _abcmake_log_note(1 "No CMakeLists.txt: ${PATH}. Skipping...")
        return()
    endif()

    message(DEBUG "[_abcmake_add_project] Adding project ${PATH}")
    _abcmake_add_subdirectory(${PATH})
    
    _abcmake_get_prop_dir(${PATH} "VERSION" version)
    set(${OUT_ABCMAKE_VER} ${version} PARENT_SCOPE)
    if (NOT version)
        _abcmake_log_warn(1 "Not abcmake: ${PATH}. Link it manually.")
    endif()
endfunction()

# _abcmake_add_project.cmake ====================================================
# ==============================================================================


# ----------------------------------------------------------------------------
# Public Functions
# ----------------------------------------------------------------------------

# ==============================================================================
# add_component.cmake ==========================================================

include(CMakeParseArguments)
set(ABC_INSTALL_LIB_SUBDIR ".")
set(ABC_INSTALL_EXE_SUBDIR ".")

# Add all projects from the components subdirectory
# @param PROCESS_LEVEL - level of the recursion
# @param TARGETNAME - name of the target to add components
function(_abcmake_add_components PROCESS_LEVEL TARGETNAME)

    # Get component directory
    _abcmake_get_components(components)

    # List of possible subprojects
    file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${components} ${CMAKE_CURRENT_SOURCE_DIR}/${components}/*)
    
    # Link all subprojects to the ${TARGETNAME}
    foreach(child ${children})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${components}/${child})
            _abcmake_target_link_components(${PROCESS_LEVEL} 
                                            ${TARGETNAME} 
                                            PATH ${CMAKE_CURRENT_SOURCE_DIR}/${components}/${child})
        endif()
    endforeach()
    
endfunction()

# Add all source files from the specified directory to the target
# @param TARGETNAME - name of the target to add sources
function(target_sources_directory TARGETNAME SOURCE_DIR)
    file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")
    message( DEBUG "[target_sources_directory] ${TARGETNAME} sources: ${SOURCES}")
    target_sources(${TARGETNAME} PRIVATE ${SOURCES})
endfunction()

# Install the target near the build directory
# @param TARGETNAME - name of the target to install
# @param DESTINATION - path to the destination directory inside the install dir
function(_abcmake_target_install TARGETNAME DESTINATION)
    # install directory
    _abcmake_get_install(install_dir)
    set (CMAKE_INSTALL_PREFIX ${install_dir}
         CACHE PATH "default install path" FORCE)
    message(DEBUG "[_abcmake_target_install] Install target: ${TARGETNAME}")
    install(TARGETS ${TARGETNAME} DESTINATION ${DESTINATION})
    
    # install include directories
    _abcmake_get_prop_dir(${CMAKE_CURRENT_SOURCE_DIR} ${ABCMAKE_DIRPROP_INCLUDE} include_dir)
    message(DEBUG "[_abcmake_target_install] Install include: ${include_dir}")
    install(DIRECTORY ${include_dir} DESTINATION ${DESTINATION})
    
endfunction()


function(_abcmake_count_parents OUT_PARENT_NUM)
    set(PARENT_NUM 0)
    get_directory_property(parent PARENT_DIRECTORY)
    while (parent)
        math(EXPR PARENT_NUM "${PARENT_NUM} + 1")
        set(parent "")
        get_directory_property(hasParent PARENT_DIRECTORY)
    endwhile()
    set(${OUT_PARENT_NUM} ${PARENT_NUM} PARENT_SCOPE)
endfunction()

# Add to the project all files from ./src, ./include, ./lib
# @param TARGETNAME - name of the target to initialize
# @param INCLUDE_DIR - path to the include directory
# @param SOURCE_DIR - path to the source directory
function(_abcmake_target_init TARGETNAME)
    set(flags)
    set(args)
    set(listArgs INCLUDE_DIR SOURCE_DIR)
    cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})

    if (NOT arg_SOURCE_DIR)
        set(arg_SOURCE_DIR "src")  # TODO: replace with _abcmake_get_src?
    endif()

    if (NOT arg_INCLUDE_DIR)
        set(arg_INCLUDE_DIR "include")  # TODO: replace with _abcmake_get_include?
    endif()

    _abcmake_count_parents(parents_num)
    set(process_level ${parents_num})
    # if no parent, print the name of the target
    if (parents_num EQUAL 0)
        _abcmake_log_header(${process_level} "${TARGETNAME}")
    endif ()
    
    # Report version
    _abcmake_set_prop_curdir(${ABCMAKE_DIRPROP_VERSION} ${ABCMAKE_VERSION})
    
    # Set name
    _abcmake_set_prop_curdir(${ABCMAKE_DIRPROP_COMPONENT_NAME} ${PROJECT_NAME})
    
    # Add target to the target list
    _abcmake_append_prop_curdir(${ABCMAKE_DIRPROP_TARGETS} ${TARGETNAME})
    
    foreach(s ${arg_SOURCE_DIR})
        target_sources_directory(${TARGETNAME} ${s})
    endforeach()
    
    target_include_directories(${TARGETNAME} PUBLIC ${arg_INCLUDE_DIR})
    _abcmake_add_components(${process_level} ${TARGETNAME})

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
        _abcmake_get_src(arg_SOURCE_DIR)
    endif()
    
    if (NOT arg_INCLUDE_DIR)
        _abcmake_get_include(arg_INCLUDE_DIR)
        if (EXISTS "include")
            set(arg_INCLUDE_DIR "include")
        else()
            set(arg_INCLUDE_DIR "")
        endif()
    endif()
    
    message(DEBUG "[add_main_component] TARGETNAME: ${TARGETNAME}")
    message(DEBUG "[add_main_component] INCLUDE_DIR: ${arg_INCLUDE_DIR}")
    message(DEBUG "[add_main_component] SOURCE_DIR: ${arg_SOURCE_DIR}")
    
    # Set Component Src and Include
    _abcmake_set_prop_curdir("${ABCMAKE_DIRPROP_SRC}" "${arg_SOURCE_DIR}")
    _abcmake_set_prop_curdir("${ABCMAKE_DIRPROP_INCLUDE}" "${arg_INCLUDE_DIR}")
    
    add_executable(${TARGETNAME})
    _abcmake_target_init(${TARGETNAME} 
                         INCLUDE_DIR ${arg_INCLUDE_DIR} 
                         SOURCE_DIR ${arg_SOURCE_DIR})
    _abcmake_target_install(${TARGETNAME} ${ABC_INSTALL_EXE_SUBDIR})
endfunction()

# Add a shared or static library component to the project
# @param TARGETNAME - name of the target to add the component
# @param INCLUDE_DIR - paths to the include directories
# @param SOURCE_DIR - paths to the source directories
# @param SHARED - if set to TRUE, the library will be shared
function(add_component TARGETNAME)
    set(flags SHARED)
    set(args)
    set(listArgs INCLUDE_DIR SOURCE_DIR)
    cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})
    
    if (NOT arg_SOURCE_DIR)
        _abcmake_get_src(arg_SOURCE_DIR)
    endif()

    if (NOT arg_INCLUDE_DIR)
        _abcmake_get_include(arg_INCLUDE_DIR)
        if (EXISTS "include")
            set(arg_INCLUDE_DIR "include")
        else()
            set(arg_INCLUDE_DIR "")
        endif()
    endif()

    if (arg_SHARED)
        add_library(${TARGETNAME} SHARED)
    else()
        add_library(${TARGETNAME} STATIC)
    endif()
    
    message(DEBUG "[add_component] TARGETNAME: ${TARGETNAME}")
    message(DEBUG "[add_component] INCLUDE_DIR: ${arg_INCLUDE_DIR}")
    message(DEBUG "[add_component] SOURCE_DIR: ${arg_SOURCE_DIR}")
    message(DEBUG "[add_component] SHARED: ${arg_SHARED}")
    
    # Set Component Src and Include
    _abcmake_set_prop_curdir("${ABCMAKE_DIRPROP_SRC}" "${arg_SOURCE_DIR}")
    _abcmake_set_prop_curdir("${ABCMAKE_DIRPROP_INCLUDE}" "${arg_INCLUDE_DIR}")
    
    _abcmake_target_init(${TARGETNAME} 
                         INCLUDE_DIR ${arg_INCLUDE_DIR} 
                         SOURCE_DIR ${arg_SOURCE_DIR})
    _abcmake_target_install(${TARGETNAME} ${ABC_INSTALL_LIB_SUBDIR})
endfunction()

# add_component.cmake ==========================================================
# ==============================================================================


# ==============================================================================
# register_components.cmake ====================================================

set(__ABCMAKE_COMPONENT_REGISTRY_SEPARATOR "::::")

# Register a component by adding it to the registry
# @param PATH - list of paths to the components
function(register_components PATH)

    foreach(path ${ARGV})
        message(DEBUG "[register_components] 📂 Path: ${path}")
        _abcmake_add_project(${path} PROJECT_ABCMAKE_VER)
        if(PROJECT_ABCMAKE_VER)
            _abcmake_get_prop_dir(${path} ${ABCMAKE_DIRPROP_COMPONENT_NAME} component_name)
            set(new_entry "${component_name}${__ABCMAKE_COMPONENT_REGISTRY_SEPARATOR}${path}")
            _abcmake_append_prop(${ABCMAKE_PROP_COMPONENT_REGISTRY} ${new_entry})
            
            _abcmake_log_header(0 "${component_name} (registered)")
        endif()
    endforeach()
    
endfunction()

# Splits into COMPONENT_ENTRY into COMPONENT_NAME and COMPONENT_PATH
function (_split_component_entry COMPONENT_ENTRY OUT_NAME OUT_PATH)
    string(FIND ${COMPONENT_ENTRY} ${__ABCMAKE_COMPONENT_REGISTRY_SEPARATOR} sep_pos)
    if(sep_pos EQUAL -1)
        message(FATAL_ERROR "Invalid component entry: ${COMPONENT_ENTRY}")
    endif()
    
    string(REGEX MATCH "^(.+)${__ABCMAKE_COMPONENT_REGISTRY_SEPARATOR}(.+)"
           ENTRY_MATCH ${COMPONENT_ENTRY})
    set(name ${CMAKE_MATCH_1})
    set(path ${CMAKE_MATCH_2})
    
    set(${OUT_NAME} ${name} PARENT_SCOPE)
    set(${OUT_PATH} ${path} PARENT_SCOPE)
endfunction()

# Gets the path of a component from the registry. Returns null if not found.
function (_abcmake_get_from_registry COMPONENT_NAME OUT_PATH)
    _abcmake_get_prop(${ABCMAKE_PROP_COMPONENT_REGISTRY} registry)
    message(DEBUG "[_abcmake_get_from_registry] Get ${COMPONENT_NAME} from : ${registry}")
    foreach(entry ${registry})
        _split_component_entry(${entry} name path)
        if(name STREQUAL COMPONENT_NAME)
            set(${OUT_PATH} ${path} PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()
    
# register_components.cmake ====================================================
# ==============================================================================



# ==============================================================================
# target_link_components.cmake =================================================

# Link a component to the target
#
# @param TARGETNAME - name of the target for linking
# @param COMPONENTPATH - path to the component to link
function (_abcmake_target_link_single_component PROCESS_LEVEL TARGETNAME COMPONENTPATH)

    _abcmake_add_project(${COMPONENTPATH} ver)
    if (ver)
        # What to link?
        _abcmake_get_prop_dir(${COMPONENTPATH} ${ABCMAKE_DIRPROP_TARGETS} to_link)
        
        # Link
        target_link_libraries(${TARGETNAME} PRIVATE ${to_link})
        _abcmake_log_ok(${PROCESS_LEVEL} "${TARGETNAME}: linked ${to_link}")
    endif()
endfunction()



# Link components to the target
# @param PROCESS_LEVEL - level of the process in the call stack (for logging)
# @param TARGETNAME - name of the target for linking
# @param PATH - paths to components to link
# @param NAME - names of components to link
function (_abcmake_target_link_components PROCESS_LEVEL TARGETNAME)
    math(EXPR process_level "${PROCESS_LEVEL} + 1")
    
    set(flags)
    set(args)
    set(listArgs PATH NAME)
    cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})
    
    message(DEBUG "[_abcmake_target_link_components] arg_PATH: ${arg_PATH}")
    message(DEBUG "[_abcmake_target_link_components] arg_NAME: ${arg_NAME}")
    
    # Link components by path
    foreach(PATH ${arg_PATH})
        _abcmake_target_link_single_component(${process_level} ${TARGETNAME} ${PATH})
    endforeach()
    
    # Link components by name
    foreach(NAME ${arg_NAME})
        set(reg_path "") # reset the variable
        _abcmake_get_from_registry(${NAME} reg_path)
        if (reg_path)
            message ( DEBUG "Found component: ${NAME} -> ${reg_path}")
            _abcmake_target_link_single_component(${process_level} ${TARGETNAME} ${reg_path})
        else()
            _abcmake_log_err(${process_level} "${NAME} not found in the component registry!")
            _abcmake_log(${process_level} "Use `register_components` to add it to the registry")
            message (FATAL_ERROR "Component ${NAME} not found in the registry")
        endif()
    endforeach()
    
    
endfunction()

# Link components to the target
# @param TARGETNAME - name of the target for linking
# @param PATH - paths to components to link
# @param NAME - names of components to link
function (target_link_components TARGETNAME)
    _abcmake_target_link_components(0 ${TARGETNAME} ${ARGN})
endfunction()

# target_link_components.cmake =================================================
# ==============================================================================

