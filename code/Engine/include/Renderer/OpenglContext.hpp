#pragma once

#include "Config.hpp"

#include <optional>
#include <string_view>

#include <Utily/Utily.hpp>

namespace Renderer {
    class OpenglContext
    {
    private:
        std::optional<GLFWwindow*> _window;
        void validate_window();

    public:
        [[nodiscard]] auto init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> Utily::Result<void, Utily::Error>;
        void poll_events();
        void stop();
        void swap_buffers() noexcept;
        [[nodiscard]] auto should_close() -> bool;

        uint_fast16_t window_width, window_height;
    };
}