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

pushd $REPO_DIR

try {
    
    echo "Configuring CMake..."
    cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug

    echo "Building project..."
    cmake --build "${BUILD_DIR}" --config Debug

    echo "Changing to build directory: ${BUILD_DIR}"
    cd "${BUILD_DIR}"

    echo "Running CTest..."
    ctest -C Debug --output-on-failure

    echo "Tests completed."

} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
}

popd
