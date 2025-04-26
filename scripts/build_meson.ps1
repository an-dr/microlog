pushd $PSScriptRoot/..

try {
    
    meson setup build --reconfigure
    meson compile -C build
    meson install -C build --destdir ../install/meson
    popd
    
} catch {
    
    Write-Host "An error occurred: $_"
    popd
    exit 1  # Exit the script with a non-zero code to indicate failure
    
}
