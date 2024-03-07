
#include "Media/Image.hpp"
#include "Profiler/Profiler.hpp"

#include <format>
#include <lodepng.h>
#include <spng.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

namespace Media {
    Image::Image(Image&& other)
        : _data(std::move(other._data))
        , _format(std::exchange(other._format, ColourFormat::rgb))
        , _width(std::exchange(other._width, 0))
        , _height(std::exchange(other._height, 0)) {
    }

    Image& Image::operator=(Image&& other) noexcept {
        if (_fence) {
            _fence.value().wait_for_sync();
        }
        if (other._fence) {
            other._fence.value().wait_for_sync();
        }
        std::swap(this->_data, other._data);
        std::swap(this->_format, other._format);
        std::swap(this->_width, other._width);
        std::swap(this->_height, other._height);
        return *this;
    }

    auto Image::init(
        std::vector<uint8_t>& encoded_png,
        bool include_alpha_channel,
        bool is_gamma_corrected) noexcept
        -> Utily::Result<void, Utily::Error> {

        Profiler::Timer timer("Media::Image::init()");

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
#if 0 // Use LodePng
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

        Profiler::Timer lode_timer("lodepng::decode()");

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
#else // Use spng
        Profiler::Timer lode_timer("spng::decode_image()");

        spng_ctx* ctx = spng_ctx_new(0);
        spng_set_png_buffer(ctx, encoded_png.data(), encoded_png.size());
        
        size_t out_size = 0;
        spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);
        _data.resize(out_size);

        spng_decode_image(ctx, _data.data(), _data.size(), SPNG_FMT_RGBA8, 0);
        spng_ctx_free(ctx);
        this->_format = Media::ColourFormat::rgba;
#endif
        _data.shrink_to_fit();
        return {};
    }

    void Image::init_raw(std::vector<uint8_t>&& raw_data, uint32_t width, uint32_t height, ColourFormat format) noexcept {
        Profiler::Timer timer("Media::Image::init_raw()");
        if (_fence) {
            Profiler::Timer fence_timer("Media::Image::fence::wait_for_sync()");
            _fence->wait_for_sync();
            _fence = std::nullopt;
        }
        _data = std::move(raw_data);
        _format = format;
        _width = width;
        _height = height;
    }

    auto Image::save_to_disk(std::filesystem::path path) noexcept
        -> Utily::Result<void, Utily::Error> {
        Profiler::Timer timer("Media::Image::save_to_disk()");

        std::vector<uint8_t> encoded;
        lodepng::State state;
        state.info_raw.colortype = LodePNGColorType::LCT_GREY;
        {
            Profiler::Timer timer("lodepng::encode()");
            if (auto error = lodepng::encode(encoded, _data, _width, _height, state); error) {
                return Utily::Error { std::string("Image.save_to_disk() failed to be converted to png: ") + lodepng_error_text(error) };
            }
        }
        {
            Profiler::Timer timer("lodepng::save_file()");
            if (auto error = lodepng::save_file(encoded, path.string()); error) {
                return Utily::Error { std::string("Image.save_to_disk() failed to save: ") + lodepng_error_text(error) };
            }
        }
        return {};
    }

    void Image::stop() noexcept {
        _data.reserve(0);
        _width = 0;
        _height = 0;
    }

    void Image::add_fence(Core::Fence&& fence) noexcept {
        _fence.emplace(std::forward<Core::Fence>(fence));
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