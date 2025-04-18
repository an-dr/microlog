pushd $PSScriptRoot
meson setup build --reconfigure --cross-file=cross-msvc.txt
meson compile -C build
popd
