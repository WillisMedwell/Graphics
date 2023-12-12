#include "Renderer/OpenglContext.hpp"

#include "Config.hpp"
#include "Util/ErrorHandling.hpp"

#include <cassert>
#include <iostream>

using namespace std::literals;

#ifdef CONFIG_TARGET_NATIVE
static void GLAPIENTRY openglDebugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam) {
    std::string sourceStr, typeStr, severityStr;

    // Decode the 'source' parameter
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        sourceStr = "Other";
        break;
    default:
        sourceStr = "Unknown";
    }

    // Decode the 'type' parameter
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "Deprecated Behavior";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "Undefined Behavior";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "Performance";
        break;
    case GL_DEBUG_TYPE_OTHER:
        typeStr = "Other";
        break;
    case GL_DEBUG_TYPE_MARKER:
        typeStr = "Marker";
        break;
    default:
        typeStr = "Unknown";
    }

    // Decode the 'severity' parameter
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        severityStr = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severityStr = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severityStr = "Low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severityStr = "Notification";
        break;
    default:
        severityStr = "Unknown";
    }

    std::cerr << "OpenGL Debug Callback:\n"
              << "Source: " << sourceStr << "\n"
              << "Type: " << typeStr << "\n"
              << "ID: " << id << "\n"
              << "Severity: " << severityStr << "\n"
              << "Message: " << message
              << "\n"
              << std::endl;

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        assert(false);
        exit(EXIT_FAILURE);
    }
}
#endif

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

namespace Renderer {
    inline std::optional<GLFWwindow*> OpenglContext::window = std::nullopt;
    inline uint_fast16_t OpenglContext::window_width { 0 }, OpenglContext::window_height { 0 };

    auto OpenglContext::init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> Util::Result<OpenglContext*, Util::ErrorMsg> {
        if (glfwInit() == GLFW_FALSE) {
            return Util::ErrorMsg("GLFW3 failed to be initialised");
        }
        if (window.has_value()) {
            return this;
        }
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);

        window = glfwCreateWindow(width, height, app_name.data(), NULL, NULL);
        if (!window) {
            return Util::ErrorMsg("GLFW3 failed to create a window");
        }
        window_width = width;
        window_height = height;
        glfwMakeContextCurrent(*window);

#ifdef CONFIG_TARGET_NATIVE
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            return Util::ErrorMsg("Glew failed to be initialised");
        }

        if constexpr (Config::DEBUG_LEVEL == Config::DebugInfo::ALL) {
            if (GLEW_ARB_debug_output) {
                glEnable(GL_DEBUG_OUTPUT);
                glDebugMessageCallback(openglDebugCallback, nullptr);
            }
        }
#endif
        glfwSetFramebufferSizeCallback(*window, framebufferSizeCallback);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        return this;
    }

    void OpenglContext::stop() {
        if (window) {
            glfwDestroyWindow(window.value());
        }
        glfwTerminate();
    }

    void OpenglContext::poll_events() {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::NONE) {
            if (!window) {
                Util::ErrorHandling::print(Util::ErrorMsg("Trying to poll events of an invalid window"));
            }
        }
        int width, height;
        glfwGetWindowSize(*window, &width, &height);
        glfwPollEvents();
        window_width = static_cast<uint_fast16_t>(width);
        window_height = static_cast<uint_fast16_t>(height);
    }
}