#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status.

BUILD_DIR="build_local_tests"

echo "Configuring CMake..."
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-Wall -Wextra -Werror"

echo "Building project..."
cmake --build "${BUILD_DIR}" --config Debug

echo "Changing to build directory: ${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Running CTest..."
ctest -C Debug --output-on-failure

echo "Tests completed."
cd .. # Go back to root

exit 0
