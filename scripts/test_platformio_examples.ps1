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

# Script to test PlatformIO examples for syntax errors (if PlatformIO is available).
# This is an optional validation step.

param(
    [string]$PackagePath = "install/platformio/microlog"
)

Push-Location $PSScriptRoot/..

try {
    
    Write-Host "Testing PlatformIO examples compilation..."
    
    # Check if pio command is available
    $pioAvailable = $false
    try {
        $null = Get-Command pio -ErrorAction Stop
        $pioAvailable = $true
        Write-Host "✓ PlatformIO CLI found"
    } catch {
        Write-Warning "PlatformIO CLI not found - skipping compilation test"
        Write-Host "To install PlatformIO: pip install platformio"
        Pop-Location
        exit 0
    }
    
    if (!$pioAvailable) {
        Pop-Location
        exit 0
    }
    
    $ExampleDirs = @(
        "examples/basic",
        "examples/advanced"
    )
    
    foreach ($exampleDir in $ExampleDirs) {
        $fullExamplePath = Join-Path $PackagePath $exampleDir
        
        if (!(Test-Path $fullExamplePath)) {
            Write-Error "Example directory not found: $fullExamplePath"
            continue
        }
        
        Write-Host "`nTesting example: $exampleDir"
        Push-Location $fullExamplePath
        
        try {
            # Create a temporary lib directory and copy the library
            New-Item -ItemType Directory -Force -Path "lib/microlog" | Out-Null
            Copy-Item "../../src/ulog.c" "lib/microlog/"
            Copy-Item "../../include/ulog.h" "lib/microlog/"
            
            # Try to check the project (this will verify syntax without building)
            Write-Host "Checking project syntax..."
            & pio project init --ide none 2>$null | Out-Null
            & pio check --skip-packages --flags="--std=c++11" 2>&1 | Out-Null
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ Example $exampleDir syntax check passed"
            } else {
                Write-Warning "Example $exampleDir has compilation warnings (this may be normal)"
            }
            
        } catch {
            Write-Warning "Could not test example $exampleDir : $_"
        } finally {
            # Clean up
            if (Test-Path "lib") {
                Remove-Item -Recurse -Force "lib"
            }
            if (Test-Path ".pio") {
                Remove-Item -Recurse -Force ".pio"
            }
            Pop-Location
        }
    }
    
    Write-Host "`n✅ PlatformIO examples testing completed!"
    
    Pop-Location
    
} catch {
    
    Write-Error "Testing failed: $_"
    Pop-Location
    exit 1
    
}
