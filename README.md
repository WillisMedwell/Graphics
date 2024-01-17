# A Portable Graphics Engine 
An experimental project exploring a range of tools, techniques, and topics related to graphics (OpenGL). Using Emscripten, the project is also portable to the web. The repo is broken into several projects:
- **Engine** is the core rendering and window management.
- **Demos** provides the logic of the app. 
- **Test** includes unit and integration tests - both native and browser based.

<details><summary><h2>Building</h2></summary>

### Prerequisites

To build this project for both native and web platforms, you need to install the following tools:

1. Install Git
   - Git is a distributed version control system.
   - You can download it from [Git's official website](https://git-scm.com/).

2. Install Vcpkg
   - vcpkg is a C++ Library Manager for Windows, Linux, and MacOS.
   - Follow the instructions on the [vcpkg GitHub repository](https://github.com/microsoft/vcpkg) to install it.

3. Install Emscripten SDK
   - Emscripten is an LLVM-to-WebAssembly compiler.
   - Installation instructions can be found on the [Emscripten website](https://emscripten.org/docs/getting_started/downloads.html).

4. Install GCC/Clang/MSVC
   - These are popular C++ compilers.
   - GCC and Clang can be installed on most Linux distributions and MacOS. 
   - MSVC (Microsoft Visual C++) can be installed as part of Visual Studio on Windows. For Windows, you can download Visual Studio from [Microsoft's website](https://visualstudio.microsoft.com/).

5. Install Ninja
   - Ninja is a small build system with a focus on speed.
   - It can be downloaded from the [Ninja Build official website](https://ninja-build.org/)

Make sure all these tools are correctly installed and configured in your system's PATH before proceeding with the project build.

---

### Windows

1. You need to change the variables in the `build-native.bat` and `build-web.bat` scripts. 
    - These variables can sometimes not be set properly by Emscripten and Vcpkg so this is the easiest solution.
    - *(Alternatively you could add them to the system's path) then remove them.*
2. Run build-native.bat
3. Run build-web.bat

---

</details>
<details><summary><h2>Libraries & Licenses</h2></summary>

- OpenGL Mathematics (GLM). *Which is licensed under The Happy Bunny License and the MIT License. <br>See [LICENSE_GLM](LICENSES/LICENSE_GLM) for more details.*

- Assimp Model Import Library. *Which is licensed under the BSD License. <br>See [LICENSE_ASSIMP](LICENSES/LICENSE_ASSIMP) for more details.*

- Google Test Framework (gtest). *Which is licensed under a revised BSD 2-Clause License. <br> See [LICENSE_GTEST](LICENSES/LICENSE_GTEST) for more details.*

- EnTT Entity Component System Library. *Which is licensed under the MIT License. <br> See [LICENSE_ENTT](LICENSES/LICENSE_ENTT) for more details.* 

- Bullet Physics SDK (bullet3). *Which is licensed under the zlib License. <br>See [LICENSE_BULLET3](LICENSES/LICENSE_BULLET3) for more details.*

- LodePNG. *Which is licensed under the zlib License. <br>See [LICENSE_LODEPNG](LICENSES/LICENSE_LODEPNG) for more details.*

- GLFW (glfw3). *Which is licensed under the zlib/libpng License. <br>See [LICENSE_GLFW](LICENSES/LICENSE_GLFW) for more details.* 

- GLEW (glew). *Which is licensed under the Modified BSD License, the Mesa 3D License (MIT License), and the Khronos License (MIT License). <br>See [LICENSE_GLEW](LICENSES/LICENSE_GLEW) for more details.*


</details>
