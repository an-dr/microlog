pushd $PSScriptRoot/..
cmake -B build -G Ninja
cmake --build build
cmake --install build
popd
