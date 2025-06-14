#!/usr/bin/env pwsh

pushd $PSScriptRoot/..

try {
    
    meson setup build --reconfigure --prefix=$PWD/install/meson/microlog
    meson compile -C build
    meson install -C build
    
    $VERSION = (Get-Content ./version).Trim()
    $SRC_FILE = "install/meson/microlog/src/ulog.c"
    $HEADER_FILE = "install/meson/microlog/include/ulog.h"
    scripts/replace_variables.ps1 $SRC_FILE $SRC_FILE @{ "ULOG_VERSION" = "$VERSION" }
    scripts/replace_variables.ps1 $HEADER_FILE $HEADER_FILE @{ "ULOG_VERSION" = "$VERSION" }
    
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
