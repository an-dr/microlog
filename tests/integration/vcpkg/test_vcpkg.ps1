# *************************************************************************
#
# Copyright (c) 2025 Andrei Gramakov. All rights reserved.
#
# This file is licensed under the terms of the MIT license.
# For a copy, see: https://opensource.org/licenses/MIT
#
# site:    https://agramakov.me e-mail:  mail@agramakov.me
#
# *************************************************************************

$ErrorActionPreference = "Stop"
Push-Location $PSScriptRoot

try {

    # Require VCPKG_ROOT
    if (-not $env:VCPKG_ROOT) {
        Write-Error "VCPKG_ROOT is not set. Please install vcpkg and set the environment variable."
    }
    $VcpkgExe = Join-Path $env:VCPKG_ROOT "vcpkg"
    $VcpkgToolchain = Join-Path $env:VCPKG_ROOT "scripts/buildsystems/vcpkg.cmake"

    $RepoRoot = (Resolve-Path "$PSScriptRoot/../../..").Path
    $OverlayPorts = Join-Path $RepoRoot "ports"

    # Install microlog via overlay port
    Write-Host "Installing microlog via overlay port..."
    & $VcpkgExe install microlog "--overlay-ports=$OverlayPorts"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "vcpkg install failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }

    # Build the consumer test project
    cmake -B ./build -DCMAKE_TOOLCHAIN_FILE="$VcpkgToolchain"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configure failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
    cmake --build ./build --verbose
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }

} catch {

    Write-Host "An error occurred: $_"
    Pop-Location
    exit 1  # Exit the script with a non-zero code to indicate failure

}

Write-Host "`n[OK] Test completed successfully."
Pop-Location
