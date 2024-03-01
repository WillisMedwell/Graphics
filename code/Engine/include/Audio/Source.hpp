#pragma once

#include "Audio/Context.hpp"
#include "Audio/Device.hpp"
#include "Audio/Buffer.hpp"

#include <cstdint>

namespace Audio {
    class Source
    {
    private:
        std::optional<uint32_t> _id = std::nullopt;
        void debug_validate();

    public:
        auto init() -> Utily::Result<void, Utily::Error>;
        auto bind(Audio::Buffer& buffer) -> Utily::Result<void, Utily::Error>;

        auto stop();

        ~Source();
    };
}
