#pragma once

#include "Audio/Buffer.hpp"
#include "Audio/Context.hpp"
#include "Audio/Device.hpp"


#include <cstdint>
#include <limits>

namespace Audio {
    class Source
    {
    private:
        constexpr static uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();

        std::optional<uint32_t> _id = std::nullopt;

        bool _has_bound_buffer = false;

    public:
        auto init() -> Utily::Result<void, Utily::Error>;
        auto bind(Audio::Buffer& buffer) -> Utily::Result<void, Utily::Error>;
        void unbind();

        auto play() -> Utily::Result<void, Utily::Error>;

        void stop();

        ~Source();
    };
}
