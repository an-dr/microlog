#!/usr/bin/env pwsh

pushd $PSScriptRoot/..

try {
    
    $INSTALL_DIR = "install/src/microlog"    
    mkdir -p $INSTALL_DIR 

    # Core files
    cp src/ulog.c $INSTALL_DIR
    cp include/ulog.h $INSTALL_DIR
    cp LICENSE $INSTALL_DIR
    cp README.md $INSTALL_DIR

    # Extensions (copy entire directory tree so future extensions are auto-included)
    if (Test-Path extensions) {
        Copy-Item -Recurse -Force extensions "$INSTALL_DIR/" | Out-Null
    }
    
    $VERSION = (Get-Content ./version).Trim()
    $SRC_FILE = "$INSTALL_DIR/ulog.c"
    $HEADER_FILE = "$INSTALL_DIR/ulog.h"
    scripts/replace_variables.ps1 $SRC_FILE $SRC_FILE @{ "ULOG_VERSION" = "$VERSION" }
    scripts/replace_variables.ps1 $HEADER_FILE $HEADER_FILE @{ "ULOG_VERSION" = "$VERSION" }

    # Future-proof: if any extension source/header later embeds @{ULOG_VERSION}@ we can substitute here.
    if (Test-Path "$INSTALL_DIR/extensions") {
        Get-ChildItem -Path "$INSTALL_DIR/extensions" -Include *.c,*.h -Recurse | ForEach-Object {
            $p = $_.FullName
            (Get-Content $p) -join "`n" | Out-Null  # touch to avoid empty pipeline warnings
            scripts/replace_variables.ps1 $p $p @{ "ULOG_VERSION" = "$VERSION" }
        }
    }
    
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
