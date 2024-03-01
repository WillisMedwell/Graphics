#pragma once

#include <optional>

#include <Utily/Utily.hpp>

namespace Audio {
    class Device
    {
    private:
        std::optional<void*> _device = std::nullopt;
        void debug_validate();

    public:
        auto init() -> Utily::Result<void, Utily::Error>;
        void stop();

        [[nodiscard]] inline auto unsafe_device_handle() -> void* {
            debug_validate();
            return _device.value_or(nullptr);
        }

        ~Device();
    };
}