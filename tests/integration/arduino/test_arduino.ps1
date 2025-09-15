#!/usr/bin/env pwsh

# Arduino Integration Test for microlog
# This test verifies that the Arduino library package is generated correctly
# and contains all required files with proper content

param(
    [switch]$Verbose = $false
)

$ErrorActionPreference = 'Stop'

function Write-TestInfo {
    param([string]$Message)
    Write-Host "[TEST] $Message" -ForegroundColor Cyan
}

function Write-TestSuccess {
    param([string]$Message)
    Write-Host "[PASS] $Message" -ForegroundColor Green
}

function Write-TestError {
    param([string]$Message)
    Write-Host "[FAIL] $Message" -ForegroundColor Red
}

function Assert-FileExists {
    param([string]$Path, [string]$Description)
    if (Test-Path $Path) {
        Write-TestSuccess "File exists: $Description"
    } else {
        Write-TestError "File missing: $Description at $Path"
        throw "Test failed: File missing"
    }
}

function Assert-FileContains {
    param([string]$Path, [string]$Pattern, [string]$Description)
    if (Test-Path $Path) {
        $content = Get-Content $Path -Raw
        if ($content -match $Pattern) {
            Write-TestSuccess "Content check: $Description"
        } else {
            Write-TestError "Content missing: $Description in $Path"
            if ($Verbose) {
                Write-Host "Expected pattern: $Pattern"
                Write-Host "Actual content (first 200 chars):"
                Write-Host $content.Substring(0, [Math]::Min(200, $content.Length))
            }
            throw "Test failed: Content missing"
        }
    } else {
        Write-TestError "File not found for content check: $Path"
        throw "Test failed: File not found"
    }
}

function Assert-DirectoryExists {
    param([string]$Path, [string]$Description)
    if (Test-Path $Path -PathType Container) {
        Write-TestSuccess "Directory exists: $Description"
    } else {
        Write-TestError "Directory missing: $Description at $Path"
        throw "Test failed: Directory missing"
    }
}

# Get paths
$scriptDir = Split-Path -Parent $PSCommandPath
$rootDir = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $scriptDir))
$buildScript = Join-Path $rootDir "scripts/build_arduino.ps1"

Write-TestInfo "Starting Arduino integration test"
Write-TestInfo "Root directory: $rootDir"

# Test 1: Verify build script exists
Assert-FileExists $buildScript "Arduino build script"

# Test 2: Run build script
Write-TestInfo "Running Arduino build script..."
try {
    & $buildScript -Clean
    Write-TestSuccess "Build script executed successfully"
} catch {
    Write-TestError "Build script failed: $_"
    throw "Test failed: Build script execution"
}

# Test 3: Verify output directory structure
$installDir = Join-Path $rootDir "install/arduino"
$libraryDir = Join-Path $installDir "microlog"

Assert-DirectoryExists $installDir "Arduino install directory"
Assert-DirectoryExists $libraryDir "Arduino library directory"
Assert-DirectoryExists (Join-Path $libraryDir "src") "Arduino library src directory"
Assert-DirectoryExists (Join-Path $libraryDir "examples") "Arduino library examples directory"
Assert-DirectoryExists (Join-Path $libraryDir "examples/BasicLogging") "Arduino example directory"

# Test 4: Verify required files exist
$requiredFiles = @{
    "library.properties" = "Arduino library metadata"
    "README.md" = "Library documentation"
    "LICENSE" = "License file"
    "keywords.txt" = "Arduino IDE keywords"
    "src/ulog.h" = "Header file"
    "src/ulog.c" = "Implementation file"
    "examples/BasicLogging/BasicLogging.ino" = "Arduino example sketch"
}

foreach ($file in $requiredFiles.Keys) {
    $filePath = Join-Path $libraryDir $file
    Assert-FileExists $filePath $requiredFiles[$file]
}

# Test 5: Verify file contents
Write-TestInfo "Verifying file contents..."

# Check library.properties
$libPropsPath = Join-Path $libraryDir "library.properties"
Assert-FileContains $libPropsPath "name=microlog" "Library name in properties"
Assert-FileContains $libPropsPath "author=Andrei Gramakov" "Author in properties"
Assert-FileContains $libPropsPath "category=Communication" "Category in properties"
Assert-FileContains $libPropsPath "includes=ulog.h" "Includes in properties"

# Check version substitution
$versionFile = Join-Path $rootDir "version"
if (Test-Path $versionFile) {
    $expectedVersion = (Get-Content $versionFile -Raw).Trim()
    Assert-FileContains $libPropsPath "version=$expectedVersion" "Version in properties"
    Assert-FileContains (Join-Path $libraryDir "src/ulog.h") "ulog v$expectedVersion" "Version in header"
    Write-TestSuccess "Version substitution verified: $expectedVersion"
}

# Check Arduino example
$examplePath = Join-Path $libraryDir "examples/BasicLogging/BasicLogging.ino"
Assert-FileContains $examplePath "#include <ulog.h>" "ulog include in example"
Assert-FileContains $examplePath "void setup\(\)" "Arduino setup function"
Assert-FileContains $examplePath "void loop\(\)" "Arduino loop function"
Assert-FileContains $examplePath "Serial\.begin" "Serial initialization"
Assert-FileContains $examplePath "ulog_info\(" "ulog function call"

# Check keywords file
$keywordsPath = Join-Path $libraryDir "keywords.txt"
Assert-FileContains $keywordsPath "ulog_info\s+KEYWORD2" "ulog_info keyword"
Assert-FileContains $keywordsPath "ulog_debug\s+KEYWORD2" "ulog_debug keyword"
Assert-FileContains $keywordsPath "ULOG_LEVEL_INFO\s+LITERAL1" "Log level constant"

# Check header file integrity
$headerPath = Join-Path $libraryDir "src/ulog.h"
Assert-FileContains $headerPath "#pragma once" "Header guard"
Assert-FileContains $headerPath "ulog_info\(" "ulog_info macro"
Assert-FileContains $headerPath "typedef enum ulog_level_enum" "Log level enum"

# Check implementation file integrity  
$implPath = Join-Path $libraryDir "src/ulog.c"
Assert-FileContains $implPath "#include.*ulog\.h" "Header include"
Assert-FileContains $implPath "void ulog_log\(" "Main log function"

# Test 6: Verify ZIP file
$versionContent = Get-Content $versionFile -Raw
$expectedVersion = $versionContent.Trim()
$zipPath = Join-Path $installDir "microlog-$expectedVersion-arduino.zip"
Assert-FileExists $zipPath "Arduino library ZIP package"

# Test 7: Verify ZIP contents (if possible)
Write-TestInfo "Verifying ZIP package contents..."
try {
    if (Get-Command Expand-Archive -ErrorAction SilentlyContinue) {
        $tempDir = if ($env:TEMP) { Join-Path $env:TEMP "arduino_test_extract" } else { Join-Path "/tmp" "arduino_test_extract" }
        if (Test-Path $tempDir) {
            Remove-Item $tempDir -Recurse -Force
        }
        New-Item -ItemType Directory -Path $tempDir -Force | Out-Null
        
        Expand-Archive -Path $zipPath -DestinationPath $tempDir -Force
        
        $extractedLibDir = Join-Path $tempDir "microlog"
        Assert-DirectoryExists $extractedLibDir "Extracted library directory"
        Assert-FileExists (Join-Path $extractedLibDir "library.properties") "Extracted library.properties"
        Assert-FileExists (Join-Path $extractedLibDir "src/ulog.h") "Extracted header file"
        
        Remove-Item $tempDir -Recurse -Force
        Write-TestSuccess "ZIP package contents verified"
    } else {
        Write-TestInfo "Expand-Archive not available, skipping ZIP content verification"
    }
} catch {
    Write-TestError "ZIP content verification failed: $_"
    throw "Test failed: ZIP content verification"
}

# Test 8: Verify Arduino library specification compliance
Write-TestInfo "Verifying Arduino library specification compliance..."

# Check that all required Arduino library files are present
$requiredArduinoFiles = @("library.properties", "src/ulog.h", "examples/BasicLogging/BasicLogging.ino")
foreach ($file in $requiredArduinoFiles) {
    Assert-FileExists (Join-Path $libraryDir $file) "Required Arduino file: $file"
}

# Check library.properties format
$libProps = Get-Content $libPropsPath
$requiredProps = @("name=", "version=", "author=", "sentence=", "category=")
foreach ($prop in $requiredProps) {
    if ($libProps | Where-Object { $_ -like "$prop*" }) {
        Write-TestSuccess "Required property found: $prop"
    } else {
        Write-TestError "Required property missing: $prop"
        throw "Test failed: Required property missing"
    }
}

Write-TestInfo "All tests completed successfully!"
Write-TestSuccess "Arduino library package is valid and ready for distribution"

return 0
