#pragma once

#include <cstdint>

namespace Config {
    enum class DebugInfo : uint_fast8_t {
        none,
        moderate, // Show shader errors, bad loads, etc.
        all // Sllow opengl callbacks on native machines.
    };
    enum class TargetPlatform : uint_fast8_t {
        web,
        native
    };

    // DebugInfo::all -> everything is checked & callbacks enabled.
    // DebugInfo::moderate -> disables opengl callbacks.
    // DebugInfo::none -> assumes all operations succeed. Disables many Utily::Error checks.
    constexpr static DebugInfo DEBUG_LEVEL = DebugInfo::all;

    // true == avoid unnecessary unbinding.
    // false == unbind explicitly, potentially good for debugging.
    constexpr static bool SKIP_UNBINDING = false;

    // false == disable analytics
    constexpr static bool SKIP_ANALYTICS = true;

    // true == potential unsafe. Fencing is not default.
    // false == ensure the gpu texture is reading valid cpu image data. Enables fencing as default.
    constexpr static bool SKIP_IMAGE_TEXTURE_FENCING = false;
    

    constexpr static bool SKIP_PROFILE = false;
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
