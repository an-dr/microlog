#!/usr/bin/env pwsh

pushd $PSScriptRoot/..

try {
    # Minimal variables
    $VCPKG = $env:VCPKG_ROOT
    if(-not $VCPKG) { throw 'VCPKG_ROOT not set' }

    $OVERLAY = "$PWD/ports"
    $TRIPLET = if($env:VCPKG_TRIPLET){ $env:VCPKG_TRIPLET } else { 'x64-windows' }

    & "$VCPKG/vcpkg" install microlog --overlay-ports=$OVERLAY --triplet=$TRIPLET

    $VERSION = (Get-Content ./version).Trim()

    # Try to locate installed include path in vcpkg for variable replacement (optional)
    $PKGROOT = Get-ChildItem -Path "$VCPKG/installed/$TRIPLET" -Directory | Where-Object { $_.Name -eq 'share' } | ForEach-Object { Join-Path $_.FullName 'microlog' }
    if(Test-Path $PKGROOT) {
        $HEADER_FILE = Join-Path (Split-Path $PKGROOT -Parent -Parent) 'include/ulog.h'
        $SRC_FILE = Join-Path (Split-Path $PKGROOT -Parent -Parent) 'src/ulog.c'
        if(Test-Path $HEADER_FILE) { scripts/replace_variables.ps1 $HEADER_FILE $HEADER_FILE @{ "ULOG_VERSION" = "$VERSION" } }
        if(Test-Path $SRC_FILE)    { scripts/replace_variables.ps1 $SRC_FILE $SRC_FILE       @{ "ULOG_VERSION" = "$VERSION" } }
    }

    popd

} catch {
    Write-Host "An error occurred: $_"
    popd
    exit 1
}
