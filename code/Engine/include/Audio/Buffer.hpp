#pragma once

#include "Audio/Context.hpp"
#include "Audio/Device.hpp"

#include "Media/Sound.hpp"

#include <cstdint>
#include <limits>

namespace Audio {
    class Source;

    class Buffer
    {
    private:
        constexpr static auto INVALID_ID = std::numeric_limits<uint32_t>::max();
        std::optional<uint32_t> _id = std::nullopt;

    public:
        [[nodiscard]] auto init() noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

        [[nodiscard]] auto load_sound(const Media::Sound& sound) -> Utily::Result<void, Utily::Error>;

        ~Buffer();

        // give access to the id of the 
        friend class Source;
    };
}
