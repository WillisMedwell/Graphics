#include "Core/OpenglContext.hpp"

#include "Config.hpp"

#include <cassert>
#include <iostream>

#include "Profiler/Profiler.hpp"

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

    std::cout << Core::DebugOpRecorder::instance().get_formatted_ops() << std::endl;

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        assert(false);
        exit(EXIT_FAILURE);
    }
}
#endif

static void framebufferSizeCallback(GLFWwindow* window [[maybe_unused]], int width, int height) {
    glViewport(0, 0, width, height);
}

namespace Core {

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
        Profiler::Timer timer("Core::OpenglContext::init()", { "OpenglContext" });

#if defined(CONFIG_TARGET_NATIVE)
        {
            Profiler::Timer timer("glfwInit()");
            if (glfwInit() == GLFW_FALSE) {
                return Utily::Error("GLFW3 failed to be initialised");
            }
        }

        {
            Profiler::Timer timer("glfwCreateWindow()");
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
            if constexpr (Config::ENABLE_VSYNC) {
                glfwSwapInterval(1);
            }
        }
#elif defined(CONFIG_TARGET_WEB)
        if (g_window) {
            _window = g_window;
            return {};
        }

        {
            Profiler::Timer timer("glfwInit()");
            if (glfwInit() == GLFW_FALSE) {
                return Utily::Error("GLFW3 failed to be initialised");
            }
        }

        {
            Profiler::Timer timer("glfwCreateWindow()");

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
        }
#endif

#ifdef CONFIG_TARGET_NATIVE
        {
            Profiler::Timer timer("glewInit()");
            glewExperimental = GL_TRUE;
            if (glewInit() != GLEW_OK) {
                return Utily::Error("Glew failed to be initialised");
            }
            if constexpr (Config::DEBUG_LEVEL == Config::DebugInfo::all) {
                if (GLEW_ARB_debug_output) {
                    Core::DebugOpRecorder::instance().clear();
                    glEnable(GL_DEBUG_OUTPUT);
                    glDebugMessageCallback(openglDebugCallback, nullptr);
                }
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
        Profiler::Timer timer("Core::OpenglContext::stop()");
        if (_window) {
            Profiler::Timer window_timer("glfwDestroyWindow()");
            glfwDestroyWindow(_window.value());
            _window = std::nullopt;
        }
        Profiler::Timer glfw_timer("glfwTerminate()");
        glfwTerminate();
        DebugOpRecorder::instance().stop();

#endif
    }

    void OpenglContext::swap_buffers() noexcept {
        Profiler::Timer timer("OpenglContext::swap_buffers()", { "OpenglContext" });
        validate_window();
        glfwSwapBuffers(*_window);
    }

    void OpenglContext::poll_events() {
        Profiler::Timer timer("OpenglContext::poll_events()", { "OpenglContext" });
        validate_window();
        {
            Profiler::Timer timer2("glfwPollEvents()", { "OpenglContext" });
            glfwPollEvents();
        }
        {
            Profiler::Timer timer3("glfwGetWindowSize()", { "OpenglContext" });
            int width, height;
            glfwGetWindowSize(*_window, &width, &height);
            window_width = static_cast<uint_fast16_t>(width);
            window_height = static_cast<uint_fast16_t>(height);
        }
    }
}