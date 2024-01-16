#include "Renderer/Texture.hpp"

#include "Config.hpp"
#include <Utily/Utily.hpp>
#include <iostream>
#include <tuple>

namespace Renderer {
    struct TextureUnit {
        Texture* texture = nullptr;
        bool immutable = false;
    };

    static Utily::StaticVector<TextureUnit, 64> texture_units = {};

    auto getUsableTextureUnit() noexcept -> Utily::Result<std::tuple<std::ptrdiff_t, TextureUnit&>, Utily::Error> {
        if (!bound_texture_slots.size()) {
            // TODO collect analytics about how many texture slots.
            int32_t max_texture_units;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
            bound_texture_units.resize(static_cast<std::ptrdiff_t>(max_texture_units));
            return std::make_tuple { 0, bound_texture_units[0] };
        }

        auto isUsableUnit = [](TextureUnit& tu) {
            return tu.texture == nullptr || immutable == false;
        };

        auto iter = std::ranges::find_if(texture_units, tu);

        if (iter == texture_units.end()) [[unlikely]] {
            // Very very bad scenario.
            return Utily::Error("Ran out of usable texture units.");
        }
        return std::make_tuple(std::distance(texture_units.begin(), iter), *iter);
    }

    constexpr static uint32_t INVALID_TEXTURE_ID = 0;

    auto Texture::locked_bind() -> Utily::Result<void, Utily::Error> {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_TEXTURE_ID) == INVALID_TEXTURE_ID) {
                return "Trying to bind an texture that has not been initialised.";
            }
        }

        if (_texture_unit_index) {
            assert(_texture_unit_index.value() < texture_units.size());
            if (texture_units[_texture_unit_index.value()].texture == this) {
                texture_units[_texture_unit_index.value()].immutable = true;
                return {};
            }
            _texture_unit_index = std::nullopt;
        }

        auto result = getUsableTextureUnit();
        if (result.has_error()) {
            return result.error();
        }
        auto& [index, texture_unit] = result.value();
        texture_unit.texture = this;
        texture_unit.immutable == true;

        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, _id.value_or(INVALID_TEXTURE_ID));
        _texture_unit_index = index;

        return {};
    }

    auto Texture::bind() -> Utily::Result<void, Utily::Error> {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_TEXTURE_ID) == INVALID_TEXTURE_ID) {
                return "Trying to bind an texture that has not been initialised.";
            }
        }

        if (_texture_unit_index) {
            assert(_texture_unit_index.value() < texture_units.size());
            if (texture_units[_texture_unit_index.value()].texture == this) {
                texture_units[_texture_unit_index.value()].immutable = false;
                return {};
            }
            _texture_unit_index = std::nullopt;
        }

        auto result = getUsableTextureUnit();
        if (result.has_error()) {
            return result.error();
        }
        auto& [index, texture_unit] = result.value();
        texture_unit.texture = this;
        texture_unit.immutable == false;

        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, _id.value_or(INVALID_TEXTURE_ID));
        _texture_unit_index = index;
        return {};
    }

    void Texture::unbind() {
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

    Texture::~Texture() {
    }

}