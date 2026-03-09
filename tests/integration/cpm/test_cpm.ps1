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

    $RepoRoot = (Resolve-Path "$PSScriptRoot/../../..").Path

    # Build the test package. Pass the repo root so CMake can use the local
    # source tree instead of fetching from a hardcoded upstream URL, which
    # would fail when running from a fork.
    cmake -B./build/cmake-cpm "-DMICROLOG_SOURCE_DIR=$RepoRoot"
    cmake --build ./build/cmake-cpm --verbose
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Tests failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
    
    Pop-Location
    
} catch {
    
    Write-Host "An error occurred: $_"
    Pop-Location
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}

Write-Host "`n[OK] Test completed successfully."
Pop-Location

