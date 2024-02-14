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

    auto Font::gen_image_atlas(uint32_t char_height_px) -> Utily::Result<Media::Image, Utily::Error> {
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
        // 2. Reduce    -> Calculate a general glyph layout for the atlas.
        auto atlas_info = std::transform_reduce(
            drawable_chars.begin(),
            drawable_chars.end(),
            cached_ft_bitmaps.begin(),
            GlyphInfo {},
            &GlyphInfo::take_max_values,
            generate_and_cache_ft_glyph);

        const size_t atlas_buffer_size = atlas_info.width * atlas_info.height * drawable_chars.size();
        std::vector<uint8_t> atlas_buffer(atlas_buffer_size);
        std::ranges::fill(atlas_buffer, (uint8_t)0);

#if 1
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(drawable_chars.size()); ++i) {
            const auto& bitmap_ft = cached_ft_bitmaps.at(i);
            uint8_t* bitmap_atlas = atlas_buffer.data() + (atlas_info.width * atlas_info.height * i);
            for (std::ptrdiff_t y = 0; y < bitmap_ft.height; ++y) {
                for (std::ptrdiff_t x = 0; x < bitmap_ft.width; ++x) {
                    const std::ptrdiff_t atlas_y = y + atlas_info.spanline - bitmap_ft.spanline;
                    const std::ptrdiff_t atlas_x = x;
                    const std::ptrdiff_t atlas_offset = atlas_info.width * atlas_y + atlas_x;
                    const std::ptrdiff_t ft_offset = y * bitmap_ft.width + x;
                    bitmap_atlas[atlas_offset] = bitmap_ft.buffer[ft_offset];
                }
            }
        }
#else
        // safer version
        auto atlas_bitmaps = atlas_buffer | std::views::chunk(atlas_info.width * atlas_info.height);
        for (auto [bitmap_ft, bitmap_atlas] : std::views::zip(cached_ft_bitmaps, atlas_bitmaps)) {
            for (std::ptrdiff_t y = 0; y < bitmap_ft.height; ++y) {
                for (std::ptrdiff_t x = 0; x < bitmap_ft.width; ++x) {
                    const std::ptrdiff_t atlas_y = y + atlas_info.spanline - bitmap_ft.spanline;
                    const std::ptrdiff_t atlas_x = x;
                    const std::ptrdiff_t atlas_offset = atlas_info.width * atlas_y + atlas_x;
                    const std::ptrdiff_t ft_offset = y * bitmap_ft.width + x;
                    bitmap_atlas[atlas_offset] = bitmap_ft.buffer[ft_offset];
                }
            }
        }
#endif
        Media::Image image;
        image.init_raw(std::move(atlas_buffer), atlas_info.width, atlas_info.height * drawable_chars.size(), Media::ColourFormat::greyscale);
        return std::move(image);
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