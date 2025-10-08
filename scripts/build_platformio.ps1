#!/usr/bin/env pwsh

# *************************************************************************
#
# Copyright (c) 2025 Andrei Gramakov. All rights reserved.
#
# This file is licensed under the terms of the MIT license.  
# For a copy, see: https://opensource.org/licenses/MIT
#
# site:    https://agramakov.me
# e-mail:  mail@agramakov.me
#
# *************************************************************************

# Script to build PlatformIO package for microlog library.
# Creates a complete PlatformIO library package with manifest, sources, and examples.

Push-Location $PSScriptRoot/..

try {
    
    $INSTALL_DIR = "install/platformio/microlog"
    
    Write-Host "Creating PlatformIO package directory structure..."
    mkdir -p $INSTALL_DIR/src
    mkdir -p $INSTALL_DIR/include
    mkdir -p $INSTALL_DIR/examples/basic
    mkdir -p $INSTALL_DIR/examples/advanced
    
    # Copy core files
    Write-Host "Copying core library files..."
    cp src/ulog.c $INSTALL_DIR/src/
    cp include/ulog.h $INSTALL_DIR/include/
    cp LICENSE $INSTALL_DIR/
    cp README.md $INSTALL_DIR/
    
    # Copy PlatformIO-specific files
    Write-Host "Copying PlatformIO manifest and examples..."
    cp platformio/library.json $INSTALL_DIR/
    cp platformio/examples/basic/* $INSTALL_DIR/examples/basic/
    cp platformio/examples/advanced/* $INSTALL_DIR/examples/advanced/
    
    # Replace version placeholders
    Write-Host "Replacing version placeholders..."
    $VERSION = (Get-Content ./version).Trim()
    $SRC_FILE = "$INSTALL_DIR/src/ulog.c"
    $HEADER_FILE = "$INSTALL_DIR/include/ulog.h"
    $MANIFEST_FILE = "$INSTALL_DIR/library.json"
    
    scripts/replace_variables.ps1 $SRC_FILE $SRC_FILE @{ "ULOG_VERSION" = "$VERSION" }
    scripts/replace_variables.ps1 $HEADER_FILE $HEADER_FILE @{ "ULOG_VERSION" = "$VERSION" }
    scripts/replace_variables.ps1 $MANIFEST_FILE $MANIFEST_FILE @{ "ULOG_VERSION" = "$VERSION" }
    
    Write-Host "PlatformIO package created successfully in: $INSTALL_DIR"
    Write-Host "Package contents:"
    Get-ChildItem -Recurse $INSTALL_DIR | ForEach-Object { Write-Host "  $_" }
    
    Pop-Location
    
} catch {
    
    Write-Host "An error occurred: $_"
    Pop-Location
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
