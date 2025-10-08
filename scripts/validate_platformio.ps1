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

# Script to validate PlatformIO package structure and content.

param(
    [string]$PackagePath = "install/platformio/microlog"
)

Push-Location $PSScriptRoot/..

try {
    
    Write-Host "Validating PlatformIO package at: $PackagePath"
    
    if (!(Test-Path $PackagePath)) {
        Write-Error "Package directory not found: $PackagePath"
        exit 1
    }
    
    # Check required files
    $RequiredFiles = @(
        "library.json",
        "src/ulog.c",
        "include/ulog.h",
        "LICENSE",
        "README.md",
        "examples/basic/main.cpp",
        "examples/basic/platformio.ini",
        "examples/advanced/main.cpp",
        "examples/advanced/platformio.ini"
    )
    
    $MissingFiles = @()
    foreach ($file in $RequiredFiles) {
        $fullPath = Join-Path $PackagePath $file
        if (!(Test-Path $fullPath)) {
            $MissingFiles += $file
        } else {
            Write-Host "✓ Found: $file"
        }
    }
    
    if ($MissingFiles.Count -gt 0) {
        Write-Error "Missing required files:"
        foreach ($file in $MissingFiles) {
            Write-Error "  - $file"
        }
        exit 1
    }
    
    # Validate library.json structure
    Write-Host "`nValidating library.json..."
    $libJsonPath = Join-Path $PackagePath "library.json"
    $libJson = Get-Content $libJsonPath | ConvertFrom-Json
    
    $RequiredFields = @("name", "version", "description", "license", "authors", "repository")
    foreach ($field in $RequiredFields) {
        if ($libJson.PSObject.Properties.Name -contains $field) {
            Write-Host "✓ library.json has field: $field"
        } else {
            Write-Error "✗ library.json missing field: $field"
            exit 1
        }
    }
    
    # Check version replacement
    if ($libJson.version -match "@.*@") {
        Write-Error "✗ Version placeholder not replaced in library.json"
        exit 1
    } else {
        Write-Host "✓ Version properly set: $($libJson.version)"
    }
    
    # Check header file version replacement
    $headerPath = Join-Path $PackagePath "include/ulog.h"
    $headerContent = Get-Content $headerPath -Raw
    if ($headerContent -match "@ULOG_VERSION@") {
        Write-Error "✗ Version placeholder not replaced in header file"
        exit 1
    } else {
        Write-Host "✓ Header file version properly replaced"
    }
    
    # Check source file version replacement
    $srcPath = Join-Path $PackagePath "src/ulog.c"
    $srcContent = Get-Content $srcPath -Raw
    if ($srcContent -match "@ULOG_VERSION@") {
        Write-Error "✗ Version placeholder not replaced in source file"
        exit 1
    } else {
        Write-Host "✓ Source file version properly replaced"
    }
    
    Write-Host "`n✅ PlatformIO package validation successful!"
    Write-Host "Package is ready for publication to PlatformIO Registry."
    
    Pop-Location
    
} catch {
    
    Write-Error "Validation failed: $_"
    Pop-Location
    exit 1
    
}
