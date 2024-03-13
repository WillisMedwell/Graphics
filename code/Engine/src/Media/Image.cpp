
#include "Media/Image.hpp"
#include "Profiler/Profiler.hpp"

#include <format>
#include <lodepng.h>
#include <spng.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

namespace Media {
    auto Image::create(std::filesystem::path path) -> Utily::Result<Image, Utily::Error> {
        Profiler::Timer timer("Media::Image::create()");
        // 1. Load the file contents into memory.
        // 2. Decode the file contents via libspng.
        // 3. Construct a valid Image instance.

        // 1.
        if (path.extension() != ".png") {
            return Utily::Error("Invalid extension for image, .png is the only supported file type.");
        }
        auto load_file_result = Utily::FileReader::load_entire_file(path);
        if (load_file_result.has_error()) {
            return load_file_result.error();
        }
        const auto& encoded_png = load_file_result.value();

        // 2.
        std::unique_ptr<uint8_t[]> data;
        size_t data_size_bytes = 0;
        glm::uvec2 dimensions = { 0, 0 };
        {
            Profiler::Timer timer2("libspng_decode_image()");
            spng_ctx* ctx = spng_ctx_new(0);
            int spng_error = 0;
            spng_error = spng_set_png_buffer(ctx, encoded_png.data(), encoded_png.size());
            if (spng_error) {
                return Utily::Error(std::string(spng_strerror(spng_error)));
            }
            spng_error = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &data_size_bytes);
            if (spng_error) {
                return Utily::Error(std::string(spng_strerror(spng_error)));
            }
            data = std::make_unique_for_overwrite<uint8_t[]>(data_size_bytes);
            spng_error = spng_decode_image(ctx, data.get(), data_size_bytes, SPNG_FMT_RGBA8, 0);
            if (spng_error) {
                return Utily::Error(std::string(spng_strerror(spng_error)));
            }
            spng_ihdr idhr;
            spng_error = spng_get_ihdr(ctx, &idhr);
            if (spng_error) {
                return Utily::Error(std::string(spng_strerror(spng_error)));
            }
            dimensions = { idhr.width, idhr.height };
            spng_ctx_free(ctx);
        }

        // 3.
        return Image(M {
            .data = std::move(data),
            .data_size_bytes = data_size_bytes,
            .dimensions = dimensions,
            .format = InternalFormat::rgba });
    }
    auto Image::create(std::span<const uint8_t> raw_bytes, glm::uvec2 dimensions, InternalFormat format) -> Utily::Result<Image, Utily::Error> {
        // 1. Check given raw sizes.
        // 2. Allocate resources.
        // 3. Copy.
        // 4. Construct valid Image instance.

        // 1.
        size_t expected_size = dimensions.x * dimensions.y;
        if (format == InternalFormat::rgba) {
            expected_size *= 4;
        } else if (format == InternalFormat::undefined) {
            return Utily::Error("Undefined format param");
        }
        if (expected_size != raw_bytes.size()) {
            return Utily::Error("The raw data is not the expected size for those dimensions and format");
        }
        // 2.
        auto data = std::make_unique_for_overwrite<uint8_t[]>(raw_bytes.size());
        auto data_size_bytes = raw_bytes.size();
        // 3.
        std::uninitialized_copy(raw_bytes.begin(), raw_bytes.end(), data.get());
        // 4.
        return Image(M {
            .data = std::move(data),
            .data_size_bytes = data_size_bytes,
            .dimensions = dimensions,
            .format = format });
    }

    auto Image::create(std::unique_ptr<uint8_t[]>&& data, size_t data_size_bytes, glm::uvec2 dimensions, InternalFormat format) -> Utily::Result<Image, Utily::Error> {
        return Image(M {
            .data = std::move(data),
            .data_size_bytes = data_size_bytes,
            .dimensions = dimensions,
            .format = format });
    }

    auto Image::opengl_format() const -> uint32_t {
        switch (_m.format) {
        case InternalFormat::greyscale:
            return GL_R8;
        case InternalFormat::rgba:
            return GL_RGBA8;
        case InternalFormat::undefined:
            [[fallthrough]];
        default:
            throw std::runtime_error("Invalid internal format enum state.");
            return std::numeric_limits<uint32_t>::max();
        }
    }

    auto Image::libspng_format() const -> uint8_t {
        switch (_m.format) {
        case InternalFormat::greyscale:
            return SPNG_COLOR_TYPE_GRAYSCALE;
        case InternalFormat::rgba:
            return SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
        case InternalFormat::undefined:
            [[fallthrough]];
        default:
            throw std::runtime_error("Invalid internal format enum state.");
            return std::numeric_limits<uint8_t>::max();
        }
    }

    Image::Image(Image&& other)
        : _m(std::move(other._m)) { }

    auto Image::save_to_disk(std::filesystem::path path) const noexcept -> Utily::Result<void, Utily::Error> {
        Profiler::Timer timer("Media::Image::save_to_disk()");

        spng_ctx* ctx = spng_ctx_new(SPNG_CTX_ENCODER);
        if (!ctx) {
            return Utily::Error("Unable to create libspng context");
        }

        spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

        spng_ihdr ihdr = {
            .width = _m.dimensions.x,
            .height = _m.dimensions.y,
            .bit_depth = 8,
            .color_type = libspng_format(),
            .compression_method = 0,
            .filter_method = 0,
            .interlace_method = 0
        };

        int spng_error = spng_set_ihdr(ctx, &ihdr);
        if (spng_error) {
            auto msg = spng_strerror(spng_error);
            spng_ctx_free(ctx);
            return Utily::Error(msg);
        }

        spng_error = spng_encode_image(ctx, _m.data.get(), _m.data_size_bytes, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
        if (spng_error) {
            auto msg = spng_strerror(spng_error);
            spng_ctx_free(ctx);
            return Utily::Error(msg);
        }

        size_t png_size = 0;
        uint8_t* png_buf = reinterpret_cast<uint8_t*>(spng_get_png_buffer(ctx, &png_size, &spng_error));

        if (!png_buf || spng_error) {
            auto msg = spng_strerror(spng_error);
            spng_ctx_free(ctx);
            return Utily::Error(msg);
        }

        auto dump_file_result = Utily::FileWriter::dump_to_file(path, std::span { png_buf, png_size });
        if (dump_file_result.has_error()) {
            return dump_file_result.error();
        }

        spng_ctx_free(ctx);
        free(png_buf);

        return {};
    }
}