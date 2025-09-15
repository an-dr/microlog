# Arduino Integration Test

This integration test verifies that the Arduino library package generation works correctly and produces a valid Arduino library.

## What it tests

1. **Build Script Execution**: Verifies that `build_arduino.ps1` runs without errors
2. **Directory Structure**: Ensures proper Arduino library directory structure is created
3. **Required Files**: Checks that all mandatory Arduino library files are present
4. **File Contents**: Validates that files contain expected content and proper formatting
5. **Version Substitution**: Verifies that version placeholders are correctly replaced
6. **Arduino Compliance**: Ensures the generated package follows Arduino library specifications
7. **ZIP Package**: Validates that the distribution ZIP file is created and contains the library

## Usage

```powershell
# Run the test
./test_arduino.ps1

# Run with verbose output
./test_arduino.ps1 -Verbose
```

## Test Structure

The test performs the following steps:

1. Locates and verifies the build script exists
2. Executes the Arduino build script with clean option
3. Validates the generated directory structure
4. Checks all required files are present
5. Verifies file contents and format
6. Tests version substitution functionality
7. Validates Arduino library specification compliance
8. Checks ZIP package creation and contents

## Expected Output

- Arduino library package in `install/arduino/microlog/`
- ZIP distribution file `install/arduino/microlog-X.Y.Z-arduino.zip`
- All files properly formatted with version substitution

## Integration with CI/CD

This test can be integrated into continuous integration pipelines to ensure Arduino library generation remains functional across code changes.
