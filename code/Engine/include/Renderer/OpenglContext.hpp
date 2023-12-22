#pragma once

#include "Config.hpp"

#include <optional>
#include <string_view>

#include "Util/Result.hpp"
#include "Util/ErrorHandling.hpp"

namespace Renderer {
    class OpenglContext
    {
    private:
        static std::optional<GLFWwindow*> window;
        static uint_fast16_t window_width, window_height;

        void validate_window();
    public:
        [[nodiscard]] auto init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> Util::Result<OpenglContext*, Util::ErrorMsg>;
        void poll_events();
        void stop();
        [[nodiscard]] auto should_close() -> bool;
    };
}