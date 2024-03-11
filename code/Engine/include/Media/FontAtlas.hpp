#pragma once

#include <Utily/Utily.hpp>

#include "Media/Image.hpp"
#include "Model/Static.hpp"
#include <array>
#include <glm/vec2.hpp>
#include <limits>
#include <ranges>
#include <utility>

namespace Media {

    class FontAtlas
    {
    public:
        /// @brief Load .ttf font from disk. Generate a font-atlas image. Can fail.
        [[nodiscard]] static auto create(std::filesystem::path path, uint32_t char_height_px) noexcept -> Utily::Result<FontAtlas, Utily::Error>;

        FontAtlas(FontAtlas&& other)
            : _m(std::move(other._m)) { }

        struct UvCoord {
            float min_x;
            float max_x;
            float min_y;
            float max_y;
        };
        [[nodiscard]] auto uv_for(char a) const noexcept -> FontAtlas::UvCoord;

        [[nodiscard]] auto atlas_image() const noexcept -> const Media::Image& { return _m.atlas_image; }
        [[nodiscard]] auto atlas_layout() const noexcept { return _m.atlas_layout; }
        [[nodiscard]] auto glyph_dimensions() const noexcept { return _m.glyph_dimensions; }

    private:
        struct M {
            Media::Image atlas_image;
            glm::vec2 atlas_layout;
            glm::vec2 glyph_dimensions;
        } _m;

        explicit FontAtlas(M&& m)
            : _m(std::move(m)) { }
    };
}
