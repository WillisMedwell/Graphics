#pragma once

#include <cstdint>

namespace Config {
    enum class DebugInfo : uint_fast8_t {
        none, 
        moderate,   // Show shader errors, bad loads, etc. 
        all         // Sllow opengl callbacks on native machines.
    };
    constexpr static DebugInfo DEBUG_LEVEL = DebugInfo::all;

    constexpr static bool SKIP_UNBINDING = false;
    constexpr static bool COLLECT_ANALYTICS = true;

    enum class TargetPlatform : uint_fast8_t {
        web,
        native
    };
}

#if defined(EMSCRIPTEN)
    #include <GLES3/gl3.h>
    #include <GLFW/glfw3.h>
    #include <emscripten/bind.h>
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
    #include <webgl/webgl1_ext.h>
    #include <webgl/webgl2_ext.h>
    namespace Config {
        static constexpr TargetPlatform PLATFORM = TargetPlatform::web;
    }
    #define CONFIG_TARGET_WEB
#endif // defined(EMSCRIPTEN)

#if !defined(EMSCRIPTEN)
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
namespace Config {
    static constexpr TargetPlatform PLATFORM = TargetPlatform::native;
}
#define CONFIG_TARGET_NATIVE
#endif // !defined(EMSCRIPTEN)
