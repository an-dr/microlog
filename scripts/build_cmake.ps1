pushd $PSScriptRoot/..

try {
    
    cmake -G "Ninja" -B./build/cmake -DCMAKE_INSTALL_PREFIX=install/cmake/microlog
    cmake --build ./build/cmake
    cmake --install ./build/cmake
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
