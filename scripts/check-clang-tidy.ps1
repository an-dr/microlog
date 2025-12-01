#!/usr/bin/env pwsh

# *************************************************************************
#
# Copyright (c) 2025 Andrei Gramakov. All rights reserved.
#
# This file is licensed under the terms of the MIT license. For a copy, see:
# https://opensource.org/licenses/MIT
#
# *************************************************************************

param(
    [switch]$Fix,
    [string]$BaseBranch = "main",
    [switch]$All,
    [switch]$Help
)

if ($Help) {
    Write-Host @"
Usage: check-clang-tidy.ps1 [OPTIONS]

Options:
  -Fix             Apply fixes automatically
  -BaseBranch      Base branch to compare against (default: main)
  -All             Check all files instead of just changed files
  -Help            Show this help message

Environment variables:
  CLANG_TIDY      Path to clang-tidy executable (default: clang-tidy)
  BUILD_DIR       Build directory for compile_commands.json (default: build/tidy)
"@
    exit 0
}

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir/.."
Push-Location $ProjectRoot

try {
    Write-Host "=== Clang-Tidy Check ===" -ForegroundColor Green
    Write-Host "Project root: $ProjectRoot"
    Write-Host "Base branch: $BaseBranch"

    # Check if clang-tidy is available
    $ClangTidy = if ($env:CLANG_TIDY) { $env:CLANG_TIDY } else { "clang-tidy" }

    try {
        $version = & $ClangTidy --version 2>&1 | Select-Object -First 1
        Write-Host "Using clang-tidy: $version"
    } catch {
        Write-Host "Error: clang-tidy not found. Please install clang-tidy." -ForegroundColor Red
        Write-Host "On Ubuntu/Debian: sudo apt-get install clang-tidy"
        Write-Host "On macOS: brew install llvm"
        Write-Host "On Windows: Install LLVM from https://llvm.org/"
        exit 1
    }

    # Get list of files to check
    $Files = @()

    if ($All) {
        Write-Host "Checking all C/C++ files" -ForegroundColor Yellow
        $Files = Get-ChildItem -Path src,include,extensions -Include *.c,*.h,*.cpp,*.hpp -Recurse -ErrorAction SilentlyContinue |
                 ForEach-Object { $_.FullName -replace [regex]::Escape($ProjectRoot + [IO.Path]::DirectorySeparatorChar), "" }
    } else {
        # Check if base branch exists
        $branchExists = git rev-parse --verify $BaseBranch 2>$null

        if ($LASTEXITCODE -eq 0) {
            Write-Host "Checking files changed compared to $BaseBranch" -ForegroundColor Yellow

            # Get committed changes on current branch
            $changedFiles = git diff --name-only "$BaseBranch...HEAD" 2>$null |
                           Where-Object { $_ -match '\.(c|h|cpp|hpp)$' }

            # Also include uncommitted changes
            $uncommittedFiles = git diff --name-only HEAD 2>$null |
                               Where-Object { $_ -match '\.(c|h|cpp|hpp)$' }

            # Combine and get unique files
            $Files = ($changedFiles + $uncommittedFiles) | Select-Object -Unique
        } else {
            Write-Host "Warning: Base branch $BaseBranch not found. Checking all files." -ForegroundColor Yellow
            $Files = Get-ChildItem -Path src,include,extensions -Include *.c,*.h,*.cpp,*.hpp -Recurse -ErrorAction SilentlyContinue |
                     ForEach-Object { $_.FullName -replace [regex]::Escape($ProjectRoot + [IO.Path]::DirectorySeparatorChar), "" }
        }
    }

    # Filter to only existing files
    $Files = $Files | Where-Object { Test-Path $_ }

    if ($Files.Count -eq 0) {
        Write-Host "No C/C++ files to check." -ForegroundColor Green
        exit 0
    }

    Write-Host "Files to check:" -ForegroundColor Yellow
    $Files | ForEach-Object { Write-Host "  - $_" }

    # Setup build directory
    $BuildDir = if ($env:BUILD_DIR) { $env:BUILD_DIR } else { "build/tidy" }
    $CompileCommands = Join-Path $BuildDir "compile_commands.json"

    # Generate compile_commands.json if it doesn't exist
    if (-not (Test-Path $CompileCommands)) {
        Write-Host "Generating compile_commands.json..." -ForegroundColor Yellow

        if (Get-Command cmake -ErrorAction SilentlyContinue) {
            New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
            cmake -B $BuildDir -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DULOG_BUILD_TESTS=OFF 2>&1 | Out-Null

            if ($LASTEXITCODE -ne 0) {
                Write-Host "Warning: Could not generate compile_commands.json with CMake" -ForegroundColor Yellow
            }
        }
    }

    # Run clang-tidy on each file
    $Errors = 0
    $Warnings = 0
    $TotalFiles = 0

    foreach ($file in $Files) {
        Write-Host ""
        Write-Host "Checking: $file" -ForegroundColor Yellow
        $TotalFiles++

        # Build the clang-tidy command arguments
        $tidyArgs = @($file)

        # Add compile commands if available
        if (Test-Path $CompileCommands) {
            $tidyArgs += "-p=$BuildDir"
        }

        # Add fix mode if requested
        if ($Fix) {
            $tidyArgs += "-fix"
        }

        # Run clang-tidy and capture output
        $output = & $ClangTidy $tidyArgs 2>&1 | Out-String

        # Check for errors and warnings
        if ($output -match "error:") {
            $Errors++
            Write-Host "✗ Found errors" -ForegroundColor Red
            Write-Host $output
        } elseif ($output -match "warning:") {
            $Warnings++
            Write-Host "⚠ Found warnings" -ForegroundColor Yellow
            Write-Host $output
        } else {
            Write-Host "✓ No issues found" -ForegroundColor Green
        }
    }

    # Print summary
    Write-Host ""
    Write-Host "=== Summary ===" -ForegroundColor Green
    Write-Host "Files checked: $TotalFiles"
    Write-Host "Files with warnings: $Warnings" -ForegroundColor Yellow
    Write-Host "Files with errors: $Errors" -ForegroundColor Red

    # Exit with error code if there were errors
    if ($Errors -gt 0) {
        Write-Host "Clang-tidy check failed with errors." -ForegroundColor Red
        exit 1
    } elseif ($Warnings -gt 0) {
        Write-Host "Clang-tidy check completed with warnings." -ForegroundColor Yellow
        exit 0
    } else {
        Write-Host "Clang-tidy check passed successfully!" -ForegroundColor Green
        exit 0
    }

} catch {
    Write-Host "An error occurred: $_" -ForegroundColor Red
    Pop-Location
    exit 1
} finally {
    Pop-Location
}
