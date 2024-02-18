#include "Media/Font.hpp"

#include <array>
#include <iostream>
#include <ranges>
// #include <mdspan>

#include <ft2build.h>
#include FT_FREETYPE_H

struct FreeType {
    FT_Library library = nullptr;

    FreeType() {
        if (auto error = FT_Init_FreeType(&library); error) {
            const auto ft_err_msg = std::string_view { FT_Error_String(error) };
            std::cerr
                << "Failed to initialise FreeType library: \n"
                << ft_err_msg
                << std::endl;
        }
    }
    ~FreeType() {
        if (library != nullptr) {
            FT_Done_FreeType(library);
        }
        library = nullptr;
    }
};

static FreeType free_type {};

namespace Media {
    auto Font::init(std::vector<uint8_t>& encoded_ttf) noexcept -> Utily::Result<void, Utily::Error> {
        FT_Face ff = nullptr;
        if (auto error = FT_New_Memory_Face(free_type.library, encoded_ttf.data(), encoded_ttf.size(), 0, &ff); error) {
            return Utily::Error { FT_Error_String(error) };
        }
        _font_face = ff;
        return {};
    }

    struct FreeTypeGlyph {
        std::vector<uint8_t> buffer;
        std::ptrdiff_t width, height, spanline;

        constexpr FreeTypeGlyph() = default;

        constexpr FreeTypeGlyph(const FT_GlyphSlot& slot)
            : width(slot->bitmap.width)
            , height(slot->bitmap.rows)
            , spanline(slot->bitmap_top) {
            auto buffer_data = std::span<uint8_t>(slot->bitmap.buffer, slot->bitmap.width * slot->bitmap.rows);
            buffer = std::vector<uint8_t> { buffer_data.begin(), buffer_data.end() };
        }
    };

    struct GlyphInfo {
        std::ptrdiff_t width = 0, height = 0, spanline = 0;

        GlyphInfo() = default;

        GlyphInfo(const FT_GlyphSlot& slot)
            : width(slot->bitmap.width)
            , height(slot->bitmap.rows + slot->bitmap_top)
            , spanline(slot->bitmap_top) { }

        static auto take_max_values(const GlyphInfo& lhs, const GlyphInfo& rhs) -> GlyphInfo {
            GlyphInfo gi;
            gi.width = std::max(lhs.width, rhs.width);
            gi.height = std::max(lhs.height, rhs.height);
            gi.spanline = std::max(lhs.spanline, rhs.spanline);
            return gi;
        }
    };

    auto Font::gen_image_atlas(uint32_t char_height_px) -> Utily::Result<std::tuple<Media::Image, int, int>, Utily::Error> {
        constexpr auto& drawable_chars = FontAtlasConstants::DRAWABLE_CHARS;

        // Validate and Scale FreeType Face.
        if (_font_face == nullptr) {
            return Utily::Error { "Font not initialised." };
        }
        FT_Face ff = reinterpret_cast<FT_Face>(_font_face);
        if (auto error = FT_Set_Pixel_Sizes(ff, char_height_px, char_height_px); error) {
            return Utily::Error { FT_Error_String(error) };
        }

        auto generate_and_cache_ft_glyph = [&](char c, FreeTypeGlyph& ftg) -> GlyphInfo {
            auto glyph_index = FT_Get_Char_Index(ff, static_cast<std::uint32_t>(c));

            if (auto error = FT_Load_Glyph(ff, glyph_index, FT_LOAD_DEFAULT); error) [[unlikely]] {
                throw std::runtime_error(FT_Error_String(error));
            }
            if (auto error = FT_Render_Glyph(ff->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL); error) [[unlikely]] {
                throw std::runtime_error(FT_Error_String(error));
            }
            ftg = FreeTypeGlyph(ff->glyph);
            return GlyphInfo(ff->glyph);
        };

        auto cached_ft_bitmaps = std::array<FreeTypeGlyph, drawable_chars.size()> {};
        // 1. Transform -> Generates and caches the freetype bitmaps & returns its glyph layout
        // 2. Reduce    -> Calculate the maximum bounding glyph layout, so all characters have enough space in atlas.
        auto atlas_info = std::transform_reduce(
            drawable_chars.begin(),
            drawable_chars.end(),
            cached_ft_bitmaps.begin(),
            GlyphInfo {},
            &GlyphInfo::take_max_values,
            generate_and_cache_ft_glyph);

        const auto [atlas_glyphs_per_row, atlas_num_rows] = [](float glyph_width, float glyph_height, float num_glyphs_in_atlas) {
#if 1 // generate most square shape by brute force.
            auto [min_diff, min_x, min_y] = std::tuple{ std::numeric_limits<float>::max(), 0, num_glyphs_in_atlas };
            for (float i = 1; i < num_glyphs_in_atlas; ++i) {
                auto x = i;
                auto y = num_glyphs_in_atlas / i;
                x = std::floor(x);
                y = std::ceil(y);
                auto diff = std::abs((x * glyph_width) - (y * glyph_height));
                if (diff < min_diff && num_glyphs_in_atlas <= x * y) {
                    min_diff = diff;
                    min_x = x;
                    min_y = y;
                }
            }
#else
            float expected_dimensions = std::sqrtf(num_glyphs_in_atlas);
            float ratio = glyph_height / glyph_width;
            float min_x = std::floor(expected_dimensions * ratio);
            float min_y = std::ceil(expected_dimensions / ratio);
            assert(num_glyphs_in_atlas < min_x * min_y);
#endif
            return std::tuple<int, int> { min_x, min_y };
        }(atlas_info.width, atlas_info.height, drawable_chars.size());

        const int atlas_img_height = atlas_info.height * atlas_num_rows;
        const int atlas_img_width = atlas_info.width * atlas_glyphs_per_row;

        std::vector<uint8_t> atlas_buffer(atlas_img_height * atlas_img_width);
        std::ranges::fill(atlas_buffer, (uint8_t)0);

        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(drawable_chars.size()); ++i) {
            const auto& bitmap_ft = cached_ft_bitmaps[i];
            const auto atlas_coords = glm::ivec2 { i % atlas_glyphs_per_row, i / atlas_glyphs_per_row };
            const auto adjusted_offset = glm::ivec2 { atlas_coords.x * atlas_info.width, atlas_coords.y * atlas_info.height };

            auto get_atlas_buffer_dest = [&](int x, int y) -> uint8_t& {
                const auto px_coords = glm::ivec2(adjusted_offset.x + x, adjusted_offset.y + y);
                return atlas_buffer[px_coords.y * atlas_img_width + px_coords.x];
            };
            for (std::ptrdiff_t y = 0; y < bitmap_ft.height; ++y) {
                for (std::ptrdiff_t x = 0; x < bitmap_ft.width; ++x) {
                    // align to relative bitmap
                    const std::ptrdiff_t relative_y = (y + atlas_info.spanline - bitmap_ft.spanline);
                    const std::ptrdiff_t ft_offset = y * bitmap_ft.width + x;
                    get_atlas_buffer_dest(x, relative_y) = bitmap_ft.buffer[ft_offset];
                }
            }
        }
        Media::Image image;
        image.init_raw(std::move(atlas_buffer), atlas_img_width, atlas_img_height, ColourFormat::greyscale);
        return std::tuple{ std::move(image), atlas_num_rows, atlas_glyphs_per_row };
    }

    void Font::stop() noexcept {
        if (_font_face != nullptr) {
            FT_Done_Face(reinterpret_cast<FT_Face>(_font_face));
        }
        _font_face = nullptr;
    }

    Font::~Font() {
        stop();
    }
}