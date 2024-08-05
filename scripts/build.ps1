pushd $PSScriptRoot/..
meson setup build --reconfigure
meson compile -C build
meson install -C build --destdir ../install
popd
