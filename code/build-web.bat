set VCPKG_PATH=C:/apps/vcpkg/vcpkg/
set EMSDK=C:/apps/emscripten/emsdk/
set EMSCRIPTEN=C:/apps/emscripten/emsdk/upstream/emscripten/

if not exist "build-web\" (
    mkdir build-web
)
cd build-web

call emcmake cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DEMSCRIPTEN=1 -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%EMSCRIPTEN%cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Release
call cmake --build . --config Release


cd ..
