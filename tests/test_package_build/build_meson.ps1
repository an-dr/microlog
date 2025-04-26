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
    pwsh -File ../../scripts/build_meson.ps1


    # Move the package to verify portability
    mkdir -p $PSScriptRoot/subprojects/microlog -ErrorAction Ignore
    mv $PSScriptRoot/../../install/meson/* $PSScriptRoot/subprojects/microlog

    meson setup build/meson --reconfigure

    meson compile -C build/meson
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}

popd
