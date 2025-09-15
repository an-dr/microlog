#!/usr/bin/env pwsh

pushd $PSScriptRoot/..

try {
    
    meson setup build --reconfigure --prefix=$PWD/install/meson/microlog
    meson compile -C build
    meson install -C build
    
    Write-Host "Meson install complete. Installation directory: $PWD/install/meson/microlog"
    
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
