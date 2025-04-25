pushd $PSScriptRoot/..
cmake -G "Ninja" -B./build/cmake -DCMAKE_INSTALL_PREFIX=install/cmake
cmake --build ./build/cmake
cmake --install ./build/cmake
popd
