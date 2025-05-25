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

$BUILD_DIR = "build_local_tests"

echo "Configuring CMake..."
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug

echo "Building project..."
cmake --build "${BUILD_DIR}" --config Debug

echo "Changing to build directory: ${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Running CTest..."
ctest -C Debug --output-on-failure

echo "Tests completed."
cd .. # Go back to root
