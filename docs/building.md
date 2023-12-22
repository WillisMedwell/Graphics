# Building
 
## Native Builds
> **Requirements** *Make sure you add them to the path too.*
> * Vcpkg - get started [here.](https://vcpkg.io/en/getting-started.html)
> 
> **Steps**
> 1. Navigate to ``/Graphics/code/``
> 2. Run ``mkdir build-native``
> 3. ``cd build-native``
> 4. Meta-build ``cmake ..``
> 5. Run-build ``cmake --build . --target Demos``

## Web Builds
> **Requirements** 
> * Vcpkg - get started [here.](https://vcpkg.io/en/getting-started.html)
> * Emscripten SDK - get started [here](https://emscripten.org/docs/getting_started/downloads.html)
> * Ninja - get it [here](https://github.com/ninja-build/ninja/releases)
> 
> **Steps**
> 1. Navigate to ``/Graphics/code/``
> 2. Open the terminal and run ``build-web.bat`` <br> *Note: Assimp requires the environment variables EMSDK and EMSCRIPTEN to be set. You will have to set them in the ``.build-web.bat`` file.*
> 3. Host the output at ``Graphics/code/build-web/Demos/`` using any local hoster of choice. For reference the python one is ``python -m http.server``
> 4. Vist localhost:8000