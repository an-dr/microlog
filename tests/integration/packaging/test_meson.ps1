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
    pwsh -File ../../../scripts/build_meson.ps1


    # Move the package to verify portability
    New-Item -ItemType Directory -Path $PSScriptRoot/subprojects -Force
    Move-Item -Force $PSScriptRoot/../../../install/meson/microlog $PSScriptRoot/subprojects

    meson setup build/meson --reconfigure

    meson compile -C build/meson
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
