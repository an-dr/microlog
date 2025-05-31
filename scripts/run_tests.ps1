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

    # Invoke the process with all supplied arguments
    & $FilePath @Arguments
    $exitCode = $LASTEXITCODE

    # If exit code is not zero, throw an exception with a clear message
    if ($exitCode -ne 0) {
        throw [System.Exception]::new(
            "Process '$FilePath' failed with exit code $exitCode." 
        )
    }
}

function Build-Tests($BUILD_DIR) {
    
    Write-Output "[TESTS] Configuring CMake..."
    Invoke-ProcessOrThrow cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DULOG_BUILD_TESTS=ON

    Write-Output "[TESTS] Building project..."
    Invoke-ProcessOrThrow cmake --build "${BUILD_DIR}" --config Debug

    Write-Output "[TESTS] Running CTest..."
    Push-Location "${BUILD_DIR}"
    Invoke-ProcessOrThrow ctest -C Debug --output-on-failure
    Pop-Location

    Write-Output "[TESTS] Completed."
}

function Clean-CoverageData($BUILD_DIR) {
    Write-Output "[COVERAGE] Cleaning previous coverage data..."
    Get-ChildItem -Path $BUILD_DIR -Recurse -Include *.gcda,*.gcov -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
    Remove-Item -Recurse -Force "${BUILD_DIR}/coverage_report" -ErrorAction SilentlyContinue
}

function Generate-CoverageReport($BUILD_DIR) {
    Write-Output "[COVERAGE] Generating coverage report..."

    Write-Output "[COVERAGE] Capturing initial coverage data..."
    Invoke-ProcessOrThrow lcov --directory $BUILD_DIR --capture --initial --output-file coverage.info --rc geninfo_unexecuted_blocks=1 --ignore-errors mismatch 2>$null

    Write-Output "[COVERAGE] Capturing coverage data..."
    Invoke-ProcessOrThrow lcov --directory $BUILD_DIR --capture --output-file coverage.info --rc geninfo_unexecuted_blocks=1 --ignore-errors mismatch 2>$null

    Write-Output "[COVERAGE] Extracting coverage for specific files..."
    Invoke-ProcessOrThrow lcov --extract coverage.info "*/src/ulog.c" --output-file coverage.info

    Write-Output "[COVERAGE] Creating coverage report directory..."
    New-Item -ItemType Directory -Force -Path "${BUILD_DIR}/coverage_report" | Out-Null

    Write-Output "[COVERAGE] Generating HTML coverage report..."
    Invoke-ProcessOrThrow genhtml coverage.info --output-directory "${BUILD_DIR}/coverage_report"

    Write-Output "[COVERAGE] Move coverage.info to coverage_report directory..."
    Move-Item -Path "coverage.info" -Destination "${BUILD_DIR}/coverage_report/coverage.info" -Force
    
    Write-Output "[COVERAGE] Coverage report generated at ${BUILD_DIR}/coverage_report/index.html"
}

# Store the current directory to return later
$CurrentDir = Get-Location

# Change to the repository directory and run the tests
Set-Location $REPO_DIR
try {
    
    Clean-CoverageData $BUILD_DIR
    Build-Tests $BUILD_DIR
    Generate-CoverageReport $BUILD_DIR

} catch {
    
    Write-Host "An error occurred: $_"
    Set-Location $CurrentDir
    exit 1  # Exit the script with a non-zero code to indicate failure
}

Set-Location $CurrentDir
Write-Host "`n[OK] Test completed successfully."
