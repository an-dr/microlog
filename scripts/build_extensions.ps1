#!/usr/bin/env pwsh

pushd $PSScriptRoot/..

try {
    
    $INSTALL_DIR = "install/extensions"    
    mkdir -p $INSTALL_DIR 
    cp LICENSE $INSTALL_DIR
    cp version $INSTALL_DIR

    ## Copy whole contents of extension directory
    cp -r extensions/* $INSTALL_DIR
    
    
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
