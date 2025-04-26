pushd $PSScriptRoot/..

try {
    
    mkdir -p install/src/microlog 
    cp src/ulog.c install/src/microlog
    cp include/ulog.h install/src/microlog
    cp LICENSE install/src/microlog
    cp README.md install/src/microlog
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
