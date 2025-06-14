pushd $PSScriptRoot/..

try {
    
    $INSTALL_DIR = "install/src/microlog"    
    mkdir -p $INSTALL_DIR 
    cp src/ulog.c $INSTALL_DIR
    cp include/ulog.h $INSTALL_DIR
    cp LICENSE $INSTALL_DIR
    cp README.md $INSTALL_DIR
    
    $VERSION = (Get-Content ./version).Trim()
    $SRC_FILE = "$INSTALL_DIR/ulog.c"
    $HEADER_FILE = "$INSTALL_DIR/ulog.h"
    scripts/replace_variables.ps1 $SRC_FILE $SRC_FILE @{ "ULOG_VERSION" = "$VERSION" }
    scripts/replace_variables.ps1 $HEADER_FILE $HEADER_FILE @{ "ULOG_VERSION" = "$VERSION" }
    
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
