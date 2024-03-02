set VCPKG_PATH=C:/apps/vcpkg/vcpkg/
set EMSDK=C:/apps/emscripten/emsdk/
set EMSCRIPTEN=C:/apps/emscripten/emsdk/upstream/emscripten/
set BUILD_TYPE=Debug

if not exist "build-web\" (
    mkdir build-web
)
cd build-web

call emcmake cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DEMSCRIPTEN=1 -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%EMSCRIPTEN%cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
call cmake --build . --target Engine --config %BUILD_TYPE%
call cmake --build . --target Demos --config %BUILD_TYPE%
@REM call cmake --build . --target Test --config %BUILD_TYPE%

cd ..
