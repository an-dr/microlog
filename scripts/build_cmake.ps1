#!/usr/bin/env pwsh

pushd $PSScriptRoot/..

try {
    
    cmake -G "Ninja" -B./build/cmake -DCMAKE_INSTALL_PREFIX=install/cmake/microlog
    cmake --build ./build/cmake
    cmake --install ./build/cmake
    
    $VERSION = (Get-Content ./version).Trim()
    $SRC_FILE = "install/cmake/microlog/src/ulog.c"
    $HEADER_FILE = "install/cmake/microlog/include/ulog.h"
    scripts/replace_variables.ps1 $SRC_FILE $SRC_FILE @{ "ULOG_VERSION" = "$VERSION" }
    scripts/replace_variables.ps1 $HEADER_FILE $HEADER_FILE @{ "ULOG_VERSION" = "$VERSION" }

    # Replace version markers inside extensions if present
    if (Test-Path "install/cmake/microlog/extensions") {
        Get-ChildItem -Path "install/cmake/microlog/extensions" -Include *.c,*.h -Recurse | ForEach-Object {
            $p = $_.FullName
            scripts/replace_variables.ps1 $p $p @{ "ULOG_VERSION" = "$VERSION" }
        }
    }

    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
