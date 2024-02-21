#pragma once

#include "Config.hpp"
#include "glm/vec2.hpp"
#include <optional>

namespace Io {
    namespace Inputs {
        enum class Keyboard {
            w = GLFW_KEY_W,
            a = GLFW_KEY_A,
            s = GLFW_KEY_S,
            d = GLFW_KEY_D
        };

        enum class Controller {
            // TODO
        };

        enum class MouseButton {
            button_left = GLFW_MOUSE_BUTTON_LEFT,
            button_right = GLFW_MOUSE_BUTTON_RIGHT,
            button_middle = GLFW_MOUSE_BUTTON_MIDDLE,
        };
    }

    namespace InputState {
        enum class Keyboard {
            released = GLFW_RELEASE,
            pressed = GLFW_PRESS,
            held = GLFW_REPEAT,
            error = GLFW_KEY_UNKNOWN
        };

        struct Mouse {
            Keyboard button_left;
            Keyboard button_middle;
            Keyboard button_right;
            glm::dvec2 position;
        };
    }

    // Works via callbacks so no need to poll.
    class InputManager
    {
    private: // callbacks
        std::optional<void*> _window;

    public:
        constexpr InputManager()
            : _window(std::nullopt) { }
        InputManager(const InputManager&) = delete;
        InputManager(InputManager&&) = delete;

        void init(void* window);
        void stop();

        auto get_key_state(Inputs::Keyboard key) const -> InputState::Keyboard;
        auto get_mouse_state() const -> const InputState::Mouse&;

        ~InputManager() {
            stop();
        }
    };

}