#include "Media/FontAtlas.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <ranges>

#include "Profiler/Profiler.hpp"

// #include <mdspan>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Media {
    constexpr static auto printable_chars = []() {
        constexpr char first_printable = char(32);
        constexpr char last_printable = char(127);
        constexpr size_t n = last_printable - first_printable;
        std::array<char, n> chars {};
        std::ranges::copy(std::views::iota(first_printable, last_printable), chars.begin());
        return chars;
    }();

    auto FontAtlas::create(std::filesystem::path path, uint32_t char_height_px) noexcept -> Utily::Result<FontAtlas, Utily::Error> {
        Profiler::Timer timer("Media::FontAtlas::create()");

        // 1. Load ttf file from disk.
        // 2. Initalise the freetype and fontface.
        // 3. Generate and cache the bitmap for each glyph.
        // 4. Determine the most compact atlas dimensions.
        // 5. Allocate raw image data.
        // 6. Blit each cached glyph bitmap onto the atlas, ensuring the same spanline.
        // 7. Create Image and font atlas.

        // 1.
        auto file_load_result = Utily::FileReader::load_entire_file(path);
        if (file_load_result.has_error()) {
            return file_load_result.error();
        }
        const auto& encoded_ttf = file_load_result.value();

        // 2.
        FT_Library free_type_library = nullptr;
        if (auto error = FT_Init_FreeType(&free_type_library); error) {
            FT_Done_FreeType(free_type_library);
            return Utily::Error { FT_Error_String(error) };
        }

        FT_Face ft_face = nullptr;
        if (auto error = FT_New_Memory_Face(free_type_library, encoded_ttf.data(), encoded_ttf.size(), 0, &ft_face); error) {
            FT_Done_FreeType(free_type_library);
            return Utily::Error { FT_Error_String(error) };
        }
        if (auto error = FT_Set_Pixel_Sizes(ft_face, 0, char_height_px); error) {
            FT_Done_FreeType(free_type_library);
            return Utily::Error { FT_Error_String(error) };
        }

        // 3.
        struct GlyphDimensions {
            glm::uvec2 bitmap_dimensions = { 0, 0 };
            uint32_t spanline = 0;
            uint32_t left_padding = 0;
        };
        struct CachedGlyph {
            char c;
            std::vector<uint8_t> bitmap;
            GlyphDimensions dimensions;
        };
        auto create_cached_glyph = [&ft_face](char c) -> CachedGlyph {
            auto glyph_index = FT_Get_Char_Index(ft_face, static_cast<std::uint32_t>(c));
            FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT);
            FT_Render_Glyph(ft_face->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
            auto ft_bitmap = std::span {
                ft_face->glyph->bitmap.buffer,
                ft_face->glyph->bitmap.width * ft_face->glyph->bitmap.rows
            };
            auto bitmap = std::vector<uint8_t>(ft_bitmap.size());
            std::ranges::copy(ft_bitmap, bitmap.begin());

            auto a = std::span {
                ft_face->glyph, static_cast<size_t>(ft_face->num_glyphs)
            };

            return CachedGlyph {
                .c = c,
                .bitmap = std::move(bitmap),
                .dimensions = {
                    .bitmap_dimensions = { ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows },
                    .spanline = static_cast<uint32_t>(ft_face->glyph->bitmap_top),
                    .left_padding = static_cast<uint32_t>(ft_face->glyph->bitmap_left) }
            };
        };
        std::array<CachedGlyph, printable_chars.size()> cached_glyphs;
        std::transform(printable_chars.begin(), printable_chars.end(), cached_glyphs.begin(), create_cached_glyph);

        FT_Done_FreeType(free_type_library);

        // 4.
        auto take_max_dimensions = [&](GlyphDimensions&& agg, const CachedGlyph& cg) {
            return GlyphDimensions {
                .bitmap_dimensions = {
                    std::max(agg.bitmap_dimensions.x, cg.dimensions.bitmap_dimensions.x),
                    std::max(agg.bitmap_dimensions.y, cg.dimensions.bitmap_dimensions.y),
                },
                .spanline = std::max(agg.spanline, cg.dimensions.spanline),
                .left_padding = std::max(agg.left_padding, cg.dimensions.left_padding),
            };
        };
        auto atlas_info = std::reduce(cached_glyphs.begin(), cached_glyphs.end(), GlyphDimensions {}, take_max_dimensions);
        auto take_max_height = [&](uint32_t&& agg, const CachedGlyph& cd) {
            return std::max(cd.dimensions.bitmap_dimensions.y + atlas_info.spanline - cd.dimensions.spanline, agg);
        };
        atlas_info.bitmap_dimensions.y = std::reduce(cached_glyphs.begin(), cached_glyphs.end(), atlas_info.bitmap_dimensions.y, take_max_height);

        // 5.
        const auto [atlas_glyphs_per_row, atlas_num_rows] = [](float glyph_width, float glyph_height, float num_glyphs_in_atlas) {
            auto [min_diff, min_x, min_y] = std::tuple { std::numeric_limits<float>::max(), 0, num_glyphs_in_atlas };
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
            return std::tuple<int, int> { min_x, min_y };
        }(atlas_info.bitmap_dimensions.x, atlas_info.bitmap_dimensions.y, printable_chars.size());

        const int atlas_img_height = atlas_info.bitmap_dimensions.y * atlas_num_rows;
        const int atlas_img_width = atlas_info.bitmap_dimensions.x * atlas_glyphs_per_row;

        auto atlas_buffer_size = static_cast<size_t>(atlas_img_height * atlas_img_width);
        auto atlas_buffer = std::make_unique<uint8_t[]>(atlas_buffer_size);
        auto atlas_buffer_begin = atlas_buffer.get();
        auto atlas_buffer_end = atlas_buffer_begin + atlas_img_height * atlas_img_width;

        std::fill(atlas_buffer_begin, atlas_buffer_end, (uint8_t)0);

        // 6.
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(printable_chars.size()); ++i) {
            const auto& bitmap_ft = cached_glyphs[i];
            const auto atlas_coords = glm::ivec2 { i % atlas_glyphs_per_row, i / atlas_glyphs_per_row };
            const auto adjusted_offset = glm::ivec2 { atlas_coords.x * atlas_info.bitmap_dimensions.x, atlas_coords.y * atlas_info.bitmap_dimensions.y };

            auto get_atlas_buffer_dest = [&](int x, int y) {
                const auto px_coords = glm::ivec2(adjusted_offset.x + x, adjusted_offset.y + y);
                return atlas_buffer_begin + (px_coords.y * atlas_img_width + px_coords.x + bitmap_ft.dimensions.left_padding);
            };
            for (std::ptrdiff_t y = 0; y < bitmap_ft.dimensions.bitmap_dimensions.y; ++y) {
                //  align to relative bitmap
                const std::ptrdiff_t relative_y = y + atlas_info.spanline - bitmap_ft.dimensions.spanline;
                const std::ptrdiff_t ft_offset = y * bitmap_ft.dimensions.bitmap_dimensions.x;

                auto src = std::span {
                    bitmap_ft.bitmap.data() + ft_offset,
                    static_cast<size_t>(bitmap_ft.dimensions.bitmap_dimensions.x)
                };
                std::ranges::copy(src, get_atlas_buffer_dest(0, relative_y));
            }
        }

        // 7.
        auto image_result = Media::Image::create(
            std::move(atlas_buffer),
            atlas_buffer_size,
            glm::uvec2(atlas_img_width, atlas_img_height),
            Media::Image::InternalFormat::greyscale);

        if (image_result.has_error()) {
            return image_result.error();
        }

        return FontAtlas(M {
            .atlas_image = std::move(image_result.value()),
            .atlas_layout = { atlas_glyphs_per_row, atlas_num_rows },
            .glyph_dimensions = {
                atlas_info.bitmap_dimensions.x,
                atlas_info.bitmap_dimensions.y },
        });
    }
    auto FontAtlas::uv_for(char a) const noexcept -> FontAtlas::UvCoord {
        assert(printable_chars.front() <= a && a <= printable_chars.back() && "Must be a printable character");

        const auto i = static_cast<int>(a - printable_chars.front());
        const float r = static_cast<float>(i / _m.atlas_layout.x);
        const float c = static_cast<float>(i % static_cast<int>(_m.atlas_layout.x));

        return {
            .min_x = c / static_cast<float>(_m.atlas_layout.x),
            .max_x = (c + 1) / static_cast<float>(_m.atlas_layout.x),
            .min_y = 1 - (r + 1) / static_cast<float>(_m.atlas_layout.y),
            .max_y = 1 - r / static_cast<float>(_m.atlas_layout.y),
        };
    }
}
