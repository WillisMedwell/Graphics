#pragma once

#include <Utily/Utily.hpp>
#include <optional>

#include "Audio/Device.hpp"

namespace Audio {
    class Context
    {
    private:
        std::optional<void*> _context = std::nullopt;
        void debug_validate();

    public:
        auto init(Audio::Device& device) -> Utily::Result<void, Utily::Error>;
        void stop();

        [[nodiscard]] inline auto unsafe_context_handle() -> void* {
            debug_validate();
            return _context.value_or(nullptr);
        }

        ~Context();
    };
}