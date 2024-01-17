
#include "Media/Image.hpp"

#include <format>
#include <lodepng.h>

namespace Media {
    Image::Image(Image&& other)
        : _data(std::move(other._data))
        , _format(std::exchange(other._format, ColourFormat::rgb))
        , _width(std::exchange(other._width, 0))
        , _height(std::exchange(other._height, 0)) {
    }

    auto Image::init(
        std::vector<uint8_t>& encoded_png,
        bool include_alpha_channel,
        bool is_gamma_corrected) noexcept
        -> Utily::Result<void, Utily::Error> {

        if (_data.size()) {
            _data.clear();
        }

        if (include_alpha_channel) {
            if (is_gamma_corrected) {
                _format = ColourFormat::s_rgba;
            } else {
                _format = ColourFormat::rgba;
            }
        } else {
            if (is_gamma_corrected) {
                _format = ColourFormat::s_rgb;
            } else {
                _format = ColourFormat::rgb;
            }
        }

        auto asLodeFormat = [](ColourFormat format) -> LodePNGColorType {
            switch (format) {
            case ColourFormat::rgb:
            case ColourFormat::s_rgb:
                return LodePNGColorType::LCT_RGB;
            case ColourFormat::rgba:
            case ColourFormat::s_rgba:
                return LodePNGColorType::LCT_RGBA;
            }
        };

        [[maybe_unused]] auto has_error = lodepng::decode(_data, _width, _height, encoded_png, asLodeFormat(_format));

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (has_error) {
                return Utily::Error {
                    std::format(
                        "Image failed to init. "
                        "The lodepng failed to decode the png file. "
                        "The error was: \n{}",
                        lodepng_error_text(has_error))
                };
            }
        }
        _data.shrink_to_fit();
        return {};
    }

    void Image::stop() noexcept {
        _data.reserve(0);
        _width = 0;
        _height = 0;
    }

    auto Image::data() noexcept -> std::optional<std::tuple<std::span<const uint8_t>, uint32_t, uint32_t, ColourFormat>> {
        if (_width == 0 || _height == 0 || _data.size() == 0) {
            return std::nullopt;
        }
        return std::tuple {
            std::span { _data.data(), _data.size() },
            _width,
            _height,
            _format
        };
    }

    void Image::add_fence(Renderer::Fence&& fence) noexcept {
        _fence.emplace(std::forward<Renderer::Fence>(fence));
    }
    
    Image::~Image() {
        if (_fence) {
            _fence->wait_for_sync();
        }
    }
}