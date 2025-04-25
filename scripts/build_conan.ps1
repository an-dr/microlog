pushd $PSScriptRoot/..

try {
    
    conan create . --build=missing
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
