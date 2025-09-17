#!/usr/bin/env pwsh

pushd $PSScriptRoot/..

try {

    cmake -G "Ninja" -B./build/abcmake -DCMAKE_INSTALL_PREFIX=install/cmake/microlog `
        -DULOG_VERSION_OVERRIDE="$(Get-Content ./version).Trim()"
    cmake --build ./build/abcmake
    cmake --install ./build/abcmake
    Write-Host "CMake install complete. Installation directory: $PWD/install/abcmake/microlog"
    
    
    
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
