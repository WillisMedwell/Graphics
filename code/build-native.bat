@echo off

set VCPKG_PATH=C:/apps/vcpkg/vcpkg/

if not exist "build-native\" (
    mkdir build-native
)
cd build-native

call cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
call cmake --build . --config Release

cd Test
call Test
cd ../Demos
call Demos