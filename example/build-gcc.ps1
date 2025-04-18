pushd $PSScriptRoot
meson setup build --reconfigure --cross-file=cross-gcc.txt
meson compile -C build
popd
