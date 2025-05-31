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

$ErrorActionPreference = "Stop"
$REPO_DIR = "$PSScriptRoot/.."
$BUILD_DIR = "build_tests"


Push-Location $REPO_DIR

function Invoke-ProcessOrThrow {
    [CmdletBinding()]
    param(
        # Path (or name) of the executable to run. Can be a full path or anything in $Env:PATH.
        [Parameter(Mandatory = $true, Position = 0)]
        [string]$FilePath,

        # Any number of arguments to pass to the executable.
        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$Arguments
    )

    # --- Invoke the process with all supplied arguments ---
    & $FilePath @Arguments
    $exitCode = $LASTEXITCODE

    # --- If exit code is not zero, throw an exception with a clear message ---
    if ($exitCode -ne 0) {
        throw [System.Exception]::new(
            "Process '$FilePath' failed with exit code $exitCode." 
        )
    }

    # Optionally, you could return the exit code or $true to indicate success:
    return $true
}


try {
    
    Write-Output "Configuring CMake..."
    Invoke-ProcessOrThrow cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DULOG_BUILD_TESTS=ON

    Write-Output "Building project..."
    Invoke-ProcessOrThrow cmake --build "${BUILD_DIR}" --config Debug

    Write-Output "Changing to build directory: ${BUILD_DIR}"
    Set-Location "${BUILD_DIR}"

    Write-Output "Running CTest..."
    Invoke-ProcessOrThrow ctest -C Debug --output-on-failure

    Write-Output "Tests completed."

} catch {
    
    Write-Host "An error occurred: $_"
    Pop-Location
    exit 1  # Exit the script with a non-zero code to indicate failure
}

Write-Host "`n[OK] Test completed successfully."
Pop-Location
