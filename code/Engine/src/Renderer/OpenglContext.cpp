#include "Renderer/OpenglContext.hpp"

#include "Config.hpp"

#include <cassert>
#include <iostream>

using namespace std::literals;

#ifdef CONFIG_TARGET_NATIVE
static void GLAPIENTRY openglDebugCallback(
    GLenum source [[maybe_unused]],
    GLenum type [[maybe_unused]],
    GLuint id [[maybe_unused]],
    GLenum severity [[maybe_unused]],
    GLsizei length [[maybe_unused]],
    const GLchar* message [[maybe_unused]],
    const void* userParam [[maybe_unused]]) {
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

static void framebufferSizeCallback(GLFWwindow* window [[maybe_unused]], int width, int height) {
    glViewport(0, 0, width, height);
}

namespace Renderer {

    void OpenglContext::validate_window() {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_window) {
                Utily::ErrorHandler::print_then_quit(Utily::Error("Invalid window handle"));
            }
        }
    }
#if defined(CONFIG_TARGET_WEB)
    static std::optional<GLFWwindow*> g_window = std::nullopt;
#endif

    auto OpenglContext::init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> Utily::Result<void, Utily::Error> {
#if defined(CONFIG_TARGET_NATIVE)
        if (glfwInit() == GLFW_FALSE) {
            return Utily::Error("GLFW3 failed to be initialised");
        }

        if (_window.has_value()) {
            return {};
        }
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);

        _window = glfwCreateWindow(width, height, app_name.data(), NULL, NULL);
        if (!_window) {
            return Utily::Error("GLFW3 failed to create a window");
        }
        window_width = width;
        window_height = height;
        glfwMakeContextCurrent(*_window);
#elif defined(CONFIG_TARGET_WEB)
        if (g_window) {
            _window = g_window;
            return {};
        }
        if (glfwInit() == GLFW_FALSE) {
            return Utily::Error("GLFW3 failed to be initialised");
        }
        if (_window.has_value()) {
            return {};
        }
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);

        _window = glfwCreateWindow(width, height, app_name.data(), NULL, NULL);
        if (!_window) {
            return Utily::Error("GLFW3 failed to create a window");
        }
        window_width = width;
        window_height = height;
        glfwMakeContextCurrent(*_window);

        g_window = _window;
#endif

#ifdef CONFIG_TARGET_NATIVE
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            return Utily::Error("Glew failed to be initialised");
        }

        if constexpr (Config::DEBUG_LEVEL == Config::DebugInfo::all) {
            if (GLEW_ARB_debug_output) {
                glEnable(GL_DEBUG_OUTPUT);
                glDebugMessageCallback(openglDebugCallback, nullptr);
            }
        }
#endif
        glfwSetFramebufferSizeCallback(*_window, framebufferSizeCallback);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        validate_window();

        

        return {};
    }

    auto OpenglContext::should_close() -> bool {
        validate_window();
        return glfwWindowShouldClose(_window.value());
    }

    void OpenglContext::stop() {
#if defined(CONFIG_TARGET_NATIVE)
        if (_window) {
            glfwDestroyWindow(_window.value());
            _window = std::nullopt;
        }
        glfwTerminate();
#endif
    }

    void OpenglContext::swap_buffers() noexcept {
        validate_window();
        glfwSwapBuffers(*_window);
    }

    void OpenglContext::poll_events() {
        validate_window();
        int width, height;
        glfwGetWindowSize(*_window, &width, &height);
        glfwPollEvents();
        window_width = static_cast<uint_fast16_t>(width);
        window_height = static_cast<uint_fast16_t>(height);
    }
}