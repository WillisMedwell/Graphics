#include "Core/Texture.hpp"
#include "Config.hpp"
#include "Core/DebugOpRecorder.hpp"
#include "Profiler/Profiler.hpp"

#include <Utily/Utily.hpp>
#include <iostream>
#include <tuple>

namespace Core {
    struct TextureUnit {
        Texture* texture = nullptr;
        bool immutable = false;
    };

    auto texture_units() -> Utily::StaticVector<TextureUnit, 64>& {
        static Utily::StaticVector<TextureUnit, 64> texture_units = {};
        return texture_units;
    }

    Texture::Texture(Texture&& other)
        : _height(std::exchange(other._height, 0))
        , _width(std::exchange(other._width, 0))
        , _id(std::exchange(other._id, std::nullopt))
        , _texture_unit_index(std::exchange(other._texture_unit_index, std::nullopt)) { }

    auto get_usable_texture_unit() noexcept
        -> Utily::Result<std::tuple<std::ptrdiff_t, TextureUnit*>, Utily::Error> {
        if (!texture_units().size()) {
            // TODO collect analytics about how many texture slots.
            int32_t max_texture_units;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
            texture_units().resize(static_cast<std::ptrdiff_t>(max_texture_units));
            return std::tuple { 0, &texture_units()[0] };
        }

        auto isUsableUnit = [](TextureUnit& tu) {
            return tu.texture == nullptr || tu.immutable == false;
        };
        auto iter = std::ranges::find_if(texture_units(), isUsableUnit);

        if (iter == texture_units().end()) [[unlikely]] {
            return Utily::Error { "Ran out of usable texture units." };
        }
        return std::tuple { std::distance(texture_units().begin(), iter), &(*iter) };
    }

    constexpr static uint32_t INVALID_TEXTURE_ID = 0;

    auto Texture::init() noexcept -> Utily::Result<void, Utily::Error> {
        Core::DebugOpRecorder::instance().push("Core::Texture", "init()");

        if (_id) {
            return Utily::Error { "Trying to override in-use Texture" };
        }
        _id = INVALID_TEXTURE_ID;
        glGenTextures(1, &_id.value());
        if (*_id == INVALID_TEXTURE_ID) {
            _id = std::nullopt;
            return Utily::Error { "Failed to create Texture. glGenTextures failed." };
        }
        return {};
    }

    auto Texture::upload_image(
        const Media::Image& image,
        Filter filter) noexcept
        -> Utily::Result<void, Utily::Error> {
        Core::DebugOpRecorder::instance().push("Core::Texture", "upload_image()");
        Profiler::Timer timer("Core::Texture::upload_image()", { "rendering" });

        if (!_id) {
            if (auto ir = init(); ir.has_error()) {
                return ir.error();
            }
        }

        if (image.raw_bytes().size() == 0) {
            return Utily::Error { "Image has no data." };
        }

        if (auto br = bind(); br.has_error()) {
            return br.error();
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int32_t)filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int32_t)filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        int32_t bytes_per_pixel = 4;
        uint32_t gl_format = GL_RGBA;

        if (image.format() == Media::Image::InternalFormat::greyscale) {
            bytes_per_pixel = 1;
            gl_format = GL_RED;
        } else {
            gl_format = GL_RGBA;
            bytes_per_pixel = 4;
        }
        const void* img_data = reinterpret_cast<const void*>(image.raw_bytes().data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, bytes_per_pixel);

        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)image.opengl_format(), image.dimensions().x, image.dimensions().y, 0, gl_format, GL_UNSIGNED_BYTE, img_data);

        return {};
    }

    auto Texture::bind(bool locked) noexcept -> Utily::Result<uint32_t, Utily::Error> {
        Core::DebugOpRecorder::instance().push("Core::Texture", "bind()");
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_TEXTURE_ID) == INVALID_TEXTURE_ID) {
                return Utily::Error { "Trying to bind an texture that has not been initialised." };
            }
        }

        if (_texture_unit_index) {
            assert(_texture_unit_index.value() < texture_units().size());
            if (texture_units()[_texture_unit_index.value()].texture == this) {
                texture_units()[_texture_unit_index.value()].immutable = locked;
                return _texture_unit_index.value();
            }
            _texture_unit_index = std::nullopt;
        }

        auto result = get_usable_texture_unit();
        if (result.has_error()) {
            return result.error();
        }
        auto& [index, texture_unit] = result.value();
        texture_unit->texture = this;
        texture_unit->immutable = locked;

        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(index));
        glBindTexture(GL_TEXTURE_2D, _id.value_or(INVALID_TEXTURE_ID));
        _texture_unit_index = static_cast<uint32_t>(index);

        return static_cast<uint32_t>(index);
    }
    void Texture::unbind() noexcept {
        Core::DebugOpRecorder::instance().push("Core::Texture", "unbind()");

        if (!texture_units().size()) {
            return;
        }

        if constexpr (Config::SKIP_UNBINDING) {
            if (texture_units()[_texture_unit_index.value_or(0)].texture == this) {
                texture_units()[_texture_unit_index.value_or(0)].immutable = false;
            }
            return;
        }

        if (texture_units()[_texture_unit_index.value_or(0)].texture == this) {
            texture_units()[_texture_unit_index.value_or(0)].texture = nullptr;
            texture_units()[_texture_unit_index.value_or(0)].immutable = false;
            glActiveTexture(GL_TEXTURE0 + _texture_unit_index.value_or(0));
            glBindTexture(GL_TEXTURE_2D, 0);
            _texture_unit_index = std::nullopt;
        }
    }

    void Texture::stop() noexcept {
        Core::DebugOpRecorder::instance().push("Core::Texture", "stop()");

        if (_id) {
            unbind();
            glDeleteTextures(1, &_id.value());
        }
        _id = std::nullopt;
        _texture_unit_index = std::nullopt;
    }

    Texture::~Texture() {
        stop();
    }

}