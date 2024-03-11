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

        [[nodiscard]] static auto create(std::filesystem::path wav_path) noexcept -> Utily::Result<Sound, Utily::Error>;

        inline static auto to_openal_format(const Format& format) -> FormatOpenal {
            if (format == Format::mono_i16) {
                return FormatOpenal::mono16;
            } else if (format == Format::stereo_i16) {
                return FormatOpenal::stereo16;
            } else {
                throw std::runtime_error("Not implemented");
            }
        }

        [[nodiscard]] inline auto raw_bytes() const noexcept -> std::span<const int16_t> { return { _m.data.cbegin(), _m.data.cend() }; }
        [[nodiscard]] inline auto frequency() const noexcept -> size_t { return _m.frequency; }
        [[nodiscard]] inline auto openal_format() const noexcept -> FormatOpenal { return to_openal_format(_m.format); }

    private:
        struct M {
            std::vector<int16_t> data;
            size_t frequency;
            Format format;
        } _m;

        explicit Sound(M&& m)
            : _m(std::move(m)) { }
    };
}