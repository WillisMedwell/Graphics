@echo off

set VCPKG_PATH=C:/apps/vcpkg/vcpkg/

if not exist "build-native\" (
    mkdir build-native
)
cd build-native

cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake
