# Generate coverage report for CMake build (GCC)
# PowerShell version
$ErrorActionPreference = "Stop"
$BUILD_DIR = "build_tests"
$REPORT_DIR = "coverage_report"

# Clean previous coverage data
Get-ChildItem -Path $BUILD_DIR -Recurse -Include *.gcda,*.gcov -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force $REPORT_DIR -ErrorAction SilentlyContinue

# Build with coverage flags
Write-Output "Configuring CMake with coverage flags..."
cmake -B $BUILD_DIR -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DULOG_BUILD_TESTS=ON -DCMAKE_C_FLAGS="--coverage" -DCMAKE_CXX_FLAGS="--coverage"

cmake --build $BUILD_DIR --config Debug

# Run tests
Push-Location $BUILD_DIR
ctest -C Debug --output-on-failure
Pop-Location

# Generate lcov report
lcov --directory $BUILD_DIR --capture --output-file coverage.info --rc geninfo_unexecuted_blocks=1

# Remove unwanted files from the coverage report
lcov --remove coverage.info '/usr/*' --output-file coverage.info

# Create the report directory if it doesn't exist
New-Item -ItemType Directory -Force -Path $REPORT_DIR | Out-Null

# Generate the HTML report, ignoring mismatched end line errors
genhtml coverage.info --output-directory $REPORT_DIR --ignore-errors mismatch

Write-Output "Coverage report generated at $REPORT_DIR/index.html"
