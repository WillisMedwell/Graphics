#pragma once

#include <bit>
#include <vector>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <exception>

#include "Config.hpp"


namespace Media {
    class Sound 
    {
    public:
        enum class Format {

        };
        enum class FormatOpenal {
            mono8 = AL_FORMAT_MONO8,
            mono16 = AL_FORMAT_MONO16,
            stereo8 = AL_FORMAT_STEREO8,
            stereo16 = AL_FORMAT_STEREO16
        };
        static auto to_openal_format(Format format) -> FormatOpenal 
        {
            throw std::runtime_error("not implemeted");
            return FormatOpenal{};
        }

        [[nodiscard]] inline auto data() const noexcept -> const uint8_t* { return _data.data(); }
        [[nodiscard]] inline auto size_in_bytes() const noexcept -> size_t { return _data_size_bytes; }
        [[nodiscard]] inline auto frequency() const noexcept -> size_t { return _frequency; }
        [[nodiscard]] inline auto openal_format() const noexcept -> FormatOpenal { return to_openal_format(_format); }

    private:
        std::vector<uint8_t> _data;
        size_t _data_size_bytes;
        size_t _frequency;
        Format _format;

    };
}