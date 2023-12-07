set VCPKG_PATH=C:/apps/vcpkg/vcpkg/
set EMSDK_PATH=C:/apps/emscripten/emsdk/
set EMSCRIPTEN=C:/apps/emscripten/emsdk/

if not exist "build-web\" (
    mkdir build-web
)
cd build-web

emcmake cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DEMSCRIPTEN=1 -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%EMSDK_PATH%upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3"

cmake --build . --target Demos 

cd ..
