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

# Build the main package
pwsh -File ../../scripts/build_cmake.ps1

# Move the package to verify portability
mkdir -p $PSScriptRoot/build/deps -ErrorAction Ignore
mv $PSScriptRoot/../../install/cmake $PSScriptRoot/build/deps/microlog


# Build the test package
cmake -B./build -DCMAKE_PREFIX_PATH="$PSScriptRoot/build/deps"
# $env:microlog_DIR = "$PSScriptRoot/build/deps/microlog" # - another option to specify the path
cmake --build ./build

popd
