@echo off

set VCPKG_PATH=C:/apps/vcpkg/vcpkg/

if not exist "build-native\" (
    mkdir build-native
)
cd build-native

call cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=Release
call cmake --build . --target Demos

call cd Demos 
call Demos