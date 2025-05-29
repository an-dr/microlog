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

$ErrorActionPreference = "Stop"
Push-Location $PSScriptRoot

try {
    
    # Build the main package
    pwsh -File ../../scripts/build_cmake.ps1

    # Move the package to verify portability
    New-Item -ItemType Directory -Path $PSScriptRoot/build/cmake/deps -Force
    Move-Item -Force $PSScriptRoot/../../install/cmake/microlog $PSScriptRoot/build/cmake/deps


    # Build the test package
    cmake -B./build/cmake -DCMAKE_PREFIX_PATH="$PSScriptRoot/build/cmake/deps"
    # $env:microlog_DIR = "$PSScriptRoot/build/cmake/deps/microlog" # - another option to specify the path
    cmake --build ./build/cmake
    
    Pop-Location
    
} catch {
    
    Write-Host "An error occurred: $_"
    Pop-Location
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}

Write-Host "`n[OK] Test completed successfully."
Pop-Location

