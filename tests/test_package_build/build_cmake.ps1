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

pwsh -File ../../scripts/build_cmake.ps1
$env:microlog_DIR = "$PSScriptRoot/../../install/cmake"

cmake -G "Ninja" -B./build
cmake --build ./build

popd
