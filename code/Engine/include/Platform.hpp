#pragma once

#include <cstdint>

enum class TargetPlatform : uint_fast8_t {
    WEB,
    NATIVE
};

#if defined(EMSCRIPTEN)
static constexpr TargetPlatform platform = TargetPlatform::WEB;

#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <webgl/webgl1_ext.h>
#include <webgl/webgl2_ext.h>

#endif // defined(EMSCRIPTEN)

#if !defined(EMSCRIPTEN)
static constexpr TargetPlatform PLATFORM = TargetPlatform::NATIVE;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#endif // !defined(EMSCRIPTEN)