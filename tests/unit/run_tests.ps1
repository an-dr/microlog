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

$REPO_DIR = "$PSScriptRoot/../.."
$BUILD_DIR = "build_tests"
$ErrorActionPreference = "Stop"

Push-Location $REPO_DIR

try {
    
    Write-Output "Configuring CMake..."
    cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug

    Write-Output "Building project..."
    cmake --build "${BUILD_DIR}" --config Debug

    Write-Output "Changing to build directory: ${BUILD_DIR}"
    Set-Location "${BUILD_DIR}"

    Write-Output "Running CTest..."
    ctest -C Debug --output-on-failure
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Tests failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }

    Write-Output "Tests completed."

} catch {
    
    Write-Host "An error occurred: $_"
    Pop-Location
    exit 1  # Exit the script with a non-zero code to indicate failure
}

Write-Host "`n[OK] Test completed successfully."
Pop-Location
