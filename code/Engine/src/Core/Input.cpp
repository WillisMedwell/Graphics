#include "Core/Input.hpp"

#include <Renderer/Renderer.hpp>
#include <Utily/Utily.hpp>

namespace Core {
    struct WindowInputs {
        InputState::Mouse mouse_state;
        std::array<InputState::Keyboard, GLFW_KEY_LAST> key_states { InputState::Keyboard::released };
    };

    // could be something that doesn't allocate.
    static std::unordered_map<GLFWwindow*, WindowInputs> window_inputs_lookup = {};

    static void mouse_position_callback(GLFWwindow* window, double x_pos, double y_pos) {
        assert(window_inputs_lookup.contains(window));
        window_inputs_lookup[window].mouse_state.position = { x_pos, y_pos };
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods [[maybe_unused]]) {
        assert(window_inputs_lookup.contains(window));
        auto& mouse_state = window_inputs_lookup[window].mouse_state;

        switch (static_cast<Inputs::MouseButton>(button)) {
        case Inputs::MouseButton::button_left:
            mouse_state.button_left = static_cast<InputState::Keyboard>(action);
            break;
        case Inputs::MouseButton::button_middle:
            mouse_state.button_middle = static_cast<InputState::Keyboard>(action);
            break;
        case Inputs::MouseButton::button_right:
            mouse_state.button_right = static_cast<InputState::Keyboard>(action);
            break;
        }
    }

    static void key_callback(GLFWwindow* window, int key, int scancode [[maybe_unused]], int action, int mods [[maybe_unused]]) {
        assert(window_inputs_lookup.contains(window));
        uint32_t ukey = static_cast<uint32_t>(key);
        if (ukey < GLFW_KEY_LAST) {
            window_inputs_lookup[window].key_states[key] = static_cast<InputState::Keyboard>(action);
        }
    }

    void InputManager::init(void* window) {
        if (_window != std::nullopt) {
            stop();
        }
        _window = window;
        GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(_window.value_or(nullptr));
        window_inputs_lookup[glfw_window] = WindowInputs {};

        glfwSetCursorPosCallback(glfw_window, mouse_position_callback);
        glfwSetMouseButtonCallback(glfw_window, mouse_button_callback);
        glfwSetKeyCallback(glfw_window, key_callback);
    }
    void InputManager::stop() {
        assert(_window != nullptr && "Probably not initalised");
        GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(_window.value_or(nullptr));

        glfwSetCursorPosCallback(glfw_window, nullptr);
        glfwSetMouseButtonCallback(glfw_window, nullptr);
        glfwSetKeyCallback(glfw_window, nullptr);

        window_inputs_lookup.erase(glfw_window);
        _window = std::nullopt;
    }

    auto InputManager::get_key_state(Inputs::Keyboard key) const -> InputState::Keyboard {
        assert(_window != nullptr && "Probably not initalised");
        GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(_window.value_or(nullptr));
        assert(window_inputs_lookup.contains(glfw_window));
        return window_inputs_lookup[glfw_window].key_states[static_cast<int>(key)];
    }

    auto InputManager::get_mouse_state() const -> const InputState::Mouse& {
        assert(_window != nullptr && "Probably not initalised");
        GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(_window.value_or(nullptr));
        assert(window_inputs_lookup.contains(glfw_window));
        return window_inputs_lookup[glfw_window].mouse_state;
    }
}