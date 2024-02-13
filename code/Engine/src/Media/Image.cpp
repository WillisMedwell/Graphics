
#include "Media/Image.hpp"

#include <format>
#include <lodepng.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

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

        if (_fence) {
            _fence->wait_for_sync();
            _fence = std::nullopt;
        }

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
            case ColourFormat::greyscale:
                return LodePNGColorType::LCT_GREY;
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

    auto Image::init_raw(
        std::vector<uint8_t>&& raw_data,
        uint32_t width,
        uint32_t height,
        ColourFormat format) noexcept
        -> Utily::Result<void, Utily::Error> {
        if (_fence) {
            _fence->wait_for_sync();
            _fence = std::nullopt;
        }
        _data = std::move(raw_data);
        _format = format;
        _width = width;
        _height = height;

        if (format == ColourFormat::greyscale && _data.size() != _width * _height) {
            return Utily::Error("Unexpected image size");
        }
        return {};
    }

    auto Image::save_to_disk(std::filesystem::path path) noexcept
        -> Utily::Result<void, Utily::Error> {
        std::vector<uint8_t> encoded;
        lodepng::State state;
        state.info_raw.colortype = LodePNGColorType::LCT_GREY;
        if (auto error = lodepng::encode(encoded, _data, _width, _height, state); error) {
            return Utily::Error { lodepng_error_text(error) };
        }
        if (auto error = lodepng::save_file(encoded, path.string()); error) {
            return Utily::Error { lodepng_error_text(error) };
        }
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

    void Image::resize(float scale) noexcept {
        this->resize(
            static_cast<uint32_t>(scale * static_cast<float>(_width)),
            static_cast<uint32_t>(scale * static_cast<float>(_height)));
    }
    void Image::resize(uint32_t width, uint32_t height) noexcept {
        int32_t channels = [&] {
            if (_format == ColourFormat::rgb || _format == ColourFormat::s_rgb) {
                return 3;
            } else {
                return 4;
            }
        }();

        auto resized_image = std::vector<uint8_t>(width * height * channels);

        stbir_resize_uint8(
            _data.data(),
            _width,
            _height,
            0,
            resized_image.data(),
            width,
            height,
            0,
            channels);

        if (_fence) {
            _fence->wait_for_sync();
            _fence = std::nullopt;
        }

        _data = std::exchange(resized_image, std::vector<uint8_t> {});
        _width = width;
        _height = height;
    }
}