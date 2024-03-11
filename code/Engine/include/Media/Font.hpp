#pragma once

#include <Utily/Utily.hpp>

#include "Media/Image.hpp"
#include "Model/Static.hpp"
#include <array>
#include <glm/vec2.hpp>
#include <limits>
#include <ranges>
#include <utility>

#if 0
namespace Media {

        namespace FontAtlasConstants {
        consteval static auto gen_drawable_chars() {
            constexpr char first_printable = char(32);
            constexpr char last_printable = char(127);
            constexpr size_t n = last_printable - first_printable;
            std::array<char, n> chars {};
            std::ranges::copy(std::views::iota(first_printable, last_printable), chars.begin());
            return chars;
        }
        constexpr static auto DRAWABLE_CHARS = gen_drawable_chars();

        consteval static auto gen_is_char_drawable_table() -> std::array<bool, 256> {
            auto table = std::array<bool, 256> { false };
            std::ptrdiff_t i = 0;
            for (auto iter = table.begin(); iter != table.end(); ++iter, ++i) {
                *iter = std::ranges::find(DRAWABLE_CHARS, static_cast<char>(i)) != DRAWABLE_CHARS.end();
            }
            return table;
        }
        constexpr static auto IS_CHAR_DRAWABLE = gen_is_char_drawable_table();
    }

    class Font;

    class FontAtlas
    {
    public:
        struct UvCoord {
            float min_x;
            float max_x;
            float min_y;
            float max_y;
        };

        Media::Image image = {};
        int32_t columns { 0 }, rows { 0 };
        int32_t glyph_width { 0 }, glyph_height { 0 };

        constexpr FontAtlas() = default;

        // Allow move operations
        FontAtlas(FontAtlas&& other) = default;
        auto operator=(FontAtlas&& other) noexcept -> FontAtlas&;

        // Refuse copy operations.
        FontAtlas(const FontAtlas&) = delete;
        FontAtlas& operator=(const FontAtlas&) = delete;

        auto init(Media::Font& font, uint32_t char_height_px) -> Utily::Result<void, Utily::Error>;

        auto uv_coord_of_char(char a) const -> UvCoord;
    };

    class Font
    {
    public:
        Font() = default;
        Font(const Font&) = delete;
        Font(Font&& other);

        [[nodiscard]] auto init(std::vector<uint8_t>& encoded_ttf) noexcept -> Utily::Result<void, Utily::Error>;
        [[nodiscard]] auto gen_image_atlas(uint32_t char_height_px) -> Utily::Result<FontAtlas, Utily::Error>;

        void stop() noexcept;

        ~Font();

    private:
        void* _font_face = nullptr;
    };

    namespace FontMeshGenerator {
        auto generate_static_mesh(std::string_view str, const float char_height, const glm::vec2 bottom_left_pos, const FontAtlas& atlas) -> std::tuple<std::array<Model::Vertex2D, 400>, std::array<Model::Index, 600>>;
    }
}

#else

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

#endif