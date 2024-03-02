#pragma once

#include <bit>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Config.hpp"

namespace Media {
    class Sound
    {
    public:
        enum class Format {
            stereo_i16,
            mono_i16,
        };
        enum class FormatOpenal {
            mono8 = AL_FORMAT_MONO8,
            mono16 = AL_FORMAT_MONO16,
            stereo8 = AL_FORMAT_STEREO8,
            stereo16 = AL_FORMAT_STEREO16
        };

        [[nodiscard]] auto init_from_wav(const std::vector<uint8_t>& encoded_wav) -> Utily::Result<void, Utily::Error>;

        inline static auto to_openal_format(const Format& format) -> FormatOpenal {
            if (format == Format::mono_i16) {
                return FormatOpenal::mono16;
            } else if (format == Format::stereo_i16) {
                return FormatOpenal::stereo16;
            } else {
                throw std::runtime_error("Not implemented");
            }
        }

        [[nodiscard]] inline auto raw_bytes() const noexcept -> std::span<const int16_t> { return { _data.cbegin(), _data.cend() }; }
        [[nodiscard]] inline auto frequency() const noexcept -> size_t { return _frequency; }
        [[nodiscard]] inline auto openal_format() const noexcept -> FormatOpenal { return to_openal_format(_format); }

    private:
        std::vector<int16_t> _data = {};
        size_t _data_size_bytes = 0;
        size_t _frequency = 0;
        Format _format = {};
    };
}