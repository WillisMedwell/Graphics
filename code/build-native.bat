@echo off

set VCPKG_PATH=C:/apps/vcpkg/vcpkg/
set BUILD_TYPE=Debug

if not exist "build-native\" (
    mkdir build-native
)

cd build-native

call cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
call cmake --build . --config %BUILD_TYPE%

@REM cd Test
@REM call Test
@REM cd ../Demos
@REM call Demos