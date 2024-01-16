#include "Renderer/Texture.hpp"

#include "Config.hpp"
#include <Utily/Utily.hpp>
#include <iostream>
#include <tuple>

constexpr static uint32_t NUM_SAMPLES = 4;

namespace Renderer {
    struct TextureUnit {
        Texture* texture = nullptr;
        bool immutable = false;
    };

    static Utily::StaticVector<TextureUnit, 64> texture_units = {};

    auto getUsableTextureUnit() noexcept
        -> Utily::Result<std::tuple<std::ptrdiff_t, TextureUnit*>, Utily::Error> {
        if (!texture_units.size()) {
            // TODO collect analytics about how many texture slots.
            int32_t max_texture_units;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
            texture_units.resize(static_cast<std::ptrdiff_t>(max_texture_units));
            return std::tuple { 0, &texture_units[0] };
        }

        auto isUsableUnit = [](TextureUnit& tu) {
            return tu.texture == nullptr || tu.immutable == false;
        };
        auto iter = std::ranges::find_if(texture_units, isUsableUnit);

        if (iter == texture_units.end()) [[unlikely]] {
            return Utily::Error { "Ran out of usable texture units." };
        }
        return std::tuple { std::distance(texture_units.begin(), iter), &(*iter) };
    }

    constexpr static uint32_t INVALID_TEXTURE_ID = 0;

    auto Texture::init() noexcept -> Utily::Result<void, Utily::Error> {
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

    auto Texture::bind(bool locked) noexcept -> Utily::Result<uint32_t, Utily::Error> {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_TEXTURE_ID) == INVALID_TEXTURE_ID) {
                return Utily::Error { "Trying to bind an texture that has not been initialised." };
            }
        }

        if (_texture_unit_index) {
            assert(_texture_unit_index.value() < texture_units.size());
            if (texture_units[_texture_unit_index.value()].texture == this) {
                texture_units[_texture_unit_index.value()].immutable = locked;
                return _texture_unit_index.value();
            }
            _texture_unit_index = std::nullopt;
        }

        auto result = getUsableTextureUnit();
        if (result.has_error()) {
            return result.error();
        }
        auto& [index, texture_unit] = result.value();
        texture_unit->texture = this;
        texture_unit->immutable = locked;

        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(index));
        glBindTexture((GLenum)_bind_type, _id.value_or(INVALID_TEXTURE_ID));
        _texture_unit_index = static_cast<uint32_t>(index);

        return static_cast<uint32_t>(index);
    }

    void Texture::setup_for_framebuffer(uint32_t width, uint32_t height, BindType sample_type) noexcept {
        if (!_id) {
            init();
        }
        _bind_type = sample_type;
        bind();

        switch (sample_type) {
        case BindType::basic: {
            glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)ColourCorrection::uncorrected, width, height, 0, GL_RGB, (GLenum)ColourCorrection::uncorrected, nullptr);
        } break;
        case BindType::multisample: {
            //glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, (GLenum)ColourCorrection::uncorrected, width, height, true);
            // /glGenRenderbuffersEXT(1, )
            //glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, 4, GL_RGBA8, 256, 256);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 256, 256);
        } break;
        }
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *_id, 0);        
    }

    void Texture::unbind() noexcept {
        if (!texture_units.size()) {
            return;
        }

        if constexpr (Config::SKIP_UNBINDING) {
            if (texture_units[_texture_unit_index.value_or(0)].texture == this) {
                texture_units[_texture_unit_index.value_or(0)].immutable = false;
            }
            return;
        }

        if (texture_units[_texture_unit_index.value_or(0)].texture == this) {
            texture_units[_texture_unit_index.value_or(0)].texture = nullptr;
            texture_units[_texture_unit_index.value_or(0)].immutable = false;
            glActiveTexture(GL_TEXTURE0 + _texture_unit_index.value_or(0));
            glBindTexture(GL_TEXTURE_2D, 0);
            _texture_unit_index = std::nullopt;
        }
    }

    void Texture::stop() noexcept {
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