#!/usr/bin/env pwsh

# Arduino Library Builder for microlog
# This script creates an Arduino library package from the microlog project
# Author: Andrei Gramakov
# License: MIT

param(
    [string]$Version = $null,
    [switch]$Clean = $false
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Get script directory and project root
$ScriptDir = Split-Path -Parent $PSCommandPath
$ProjectRoot = Split-Path -Parent $ScriptDir

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Green
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

Write-Info "Building Arduino library for microlog"
Write-Info "Project root: $ProjectRoot"

# Read version from version file if not specified
if (-not $Version) {
    $VersionFile = Join-Path $ProjectRoot "version"
    if (Test-Path $VersionFile) {
        $Version = (Get-Content $VersionFile -Raw).Trim()
        Write-Info "Version from file: $Version"
    } else {
        Write-Error "Version file not found and no version specified"
        exit 1
    }
}

# Validate version format
if ($Version -notmatch '^\d+\.\d+\.\d+') {
    Write-Error "Invalid version format: $Version (expected: X.Y.Z)"
    exit 1
}

# Set up paths
$OutputDir = Join-Path $ProjectRoot "install/arduino"
if (-not (Test-Path $OutputDir)) {
    Write-Info "Creating output directory: $OutputDir"
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}
$OutputPath = Resolve-Path $OutputDir
$LibraryName = "microlog"
$LibraryDir = Join-Path $OutputPath $LibraryName

Write-Info "Output directory: $OutputPath"
Write-Info "Library directory: $LibraryDir"

# Clean if requested
if ($Clean -and (Test-Path $LibraryDir)) {
    Write-Info "Cleaning existing library directory"
    Remove-Item $LibraryDir -Recurse -Force
}

# Create library directory structure
Write-Info "Creating library directory structure"
New-Item -ItemType Directory -Path $LibraryDir -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $LibraryDir "src") -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $LibraryDir "examples") -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $LibraryDir "examples/BasicLogging") -Force | Out-Null

# Copy and process library.properties
Write-Info "Creating library.properties"
$LibraryPropsTemplate = Join-Path $ProjectRoot "arduino/library.properties"
$LibraryPropsContent = Get-Content $LibraryPropsTemplate -Raw
$LibraryPropsContent = $LibraryPropsContent -replace '@ULOG_VERSION@', $Version
Set-Content -Path (Join-Path $LibraryDir "library.properties") -Value $LibraryPropsContent

# Copy keywords.txt
Write-Info "Copying keywords.txt"
Copy-Item (Join-Path $ProjectRoot "arduino/keywords.txt") (Join-Path $LibraryDir "keywords.txt")

# Copy and process source files
Write-Info "Copying source files"
$SourceHeader = Join-Path $ProjectRoot "include/ulog.h"
$SourceImpl = Join-Path $ProjectRoot "src/ulog.c"

if (-not (Test-Path $SourceHeader)) {
    Write-Error "Source header not found: $SourceHeader"
    exit 1
}

if (-not (Test-Path $SourceImpl)) {
    Write-Error "Source implementation not found: $SourceImpl"
    exit 1
}

# Process header file (replace version placeholder)
$HeaderContent = Get-Content $SourceHeader -Raw
$HeaderContent = $HeaderContent -replace '@ULOG_VERSION@', $Version
Set-Content -Path (Join-Path $LibraryDir "src/ulog.h") -Value $HeaderContent

# Process implementation file (replace version placeholder)
$ImplContent = Get-Content $SourceImpl -Raw
$ImplContent = $ImplContent -replace '@ULOG_VERSION@', $Version
Set-Content -Path (Join-Path $LibraryDir "src/ulog.c") -Value $ImplContent

# Copy documentation files
Write-Info "Copying documentation"
Copy-Item (Join-Path $ProjectRoot "README.md") (Join-Path $LibraryDir "README.md")
Copy-Item (Join-Path $ProjectRoot "LICENSE") (Join-Path $LibraryDir "LICENSE")

# Create basic Arduino example
Write-Info "Creating Arduino example"
$ExampleTemplate = Join-Path $ProjectRoot "arduino/BasicLogging.ino.template"

if (-not (Test-Path $ExampleTemplate)) {
    Write-Error "Example template not found: $ExampleTemplate"
    exit 1
}

$ExampleContent = Get-Content $ExampleTemplate -Raw
Set-Content -Path (Join-Path $LibraryDir "examples/BasicLogging/BasicLogging.ino") -Value $ExampleContent

Write-Info "Arduino library package created successfully!"
Write-Info "Library location: $LibraryDir"
Write-Info ""
Write-Info "To install in Arduino IDE:"
Write-Info "1. Copy the '$LibraryName' folder to your Arduino libraries directory, or"
Write-Info "2. Create a ZIP file and install via Sketch > Include Library > Add .ZIP Library"
Write-Info ""
Write-Info "Arduino libraries directory locations:"
Write-Info "  Windows: %USERPROFILE%\Documents\Arduino\libraries\"
Write-Info "  macOS:   ~/Documents/Arduino/libraries/"
Write-Info "  Linux:   ~/Arduino/libraries/"

# Create ZIP file for easy distribution
$ZipPath = Join-Path $OutputPath "$LibraryName-$Version-arduino.zip"
Write-Info "Creating ZIP package: $ZipPath"

try {
    if (Test-Path $ZipPath) {
        Remove-Item $ZipPath -Force
    }
    
    # Use .NET compression
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory($LibraryDir, $ZipPath)
    
    Write-Info "ZIP package created successfully!"
} catch {
    Write-Warning "Failed to create ZIP package: $_"
    Write-Info "You can manually create a ZIP file from the library directory"
}

Write-Info "Build completed successfully!"
