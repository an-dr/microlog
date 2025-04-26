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

pushd $PSScriptRoot

try {
    
    # Build the main package
    pwsh -File ../../scripts/build_cmake.ps1

    # Move the package to verify portability
    mkdir -p $PSScriptRoot/build/cmake/deps -ErrorAction Ignore
    mv -Force $PSScriptRoot/../../install/cmake/microlog $PSScriptRoot/build/cmake/deps


    # Build the test package
    cmake -B./build/cmake -DCMAKE_PREFIX_PATH="$PSScriptRoot/build/cmake/deps"
    # $env:microlog_DIR = "$PSScriptRoot/build/cmake/deps/microlog" # - another option to specify the path
    cmake --build ./build/cmake
    
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}

