#include "Media/Font.hpp"

#include <ft2build.h>
#include <iostream>

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

    auto Font::gen_image_atlas(uint32_t char_height_px) -> Utily::Result<Media::Image, Utily::Error> {
        if (_font_face == nullptr) {
            return Utily::Error { "Font not initialised." };
        }
        FT_Face ff = reinterpret_cast<FT_Face>(_font_face);

        if (auto error = FT_Set_Pixel_Sizes(ff, char_height_px, char_height_px); error) {
            return Utily::Error { FT_Error_String(error) };
        }

        int ff_bm_max_top = 0;
        int ff_bm_max_spanline_dist = 0;
        int ff_bm_max_width = 0;

        struct FreeTypeGlyph {
            std::vector<uint8_t> buffer;
            int width, height;
            int left, top;
        };

        constexpr size_t N = FontAtlasConstants::DRAWABLE_CHARS.size();
        Utily::StaticVector<FreeTypeGlyph, N> ff_glyphs;

        for (const char& c : FontAtlasConstants::DRAWABLE_CHARS) {
            auto glyph_index = FT_Get_Char_Index(ff, static_cast<std::uint32_t>(c));

            if (auto error = FT_Load_Glyph(ff, glyph_index, FT_LOAD_DEFAULT); error) [[unlikely]] {
                return Utily::Error { FT_Error_String(error) };
            }
            if (auto error = FT_Render_Glyph(ff->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL); error) [[unlikely]] {
                return Utily::Error { FT_Error_String(error) };
            }

            const int curr_spanline_dist = ff->glyph->bitmap.rows - ff->glyph->bitmap_top;
            const int curr_width = ff->glyph->bitmap.width + ff->glyph->bitmap_left;
            const int curr_top = ff->glyph->bitmap_top;

            ff_bm_max_width = (curr_width > ff_bm_max_width) ? curr_width : ff_bm_max_width;
            ff_bm_max_top = (curr_top > ff_bm_max_top) ? curr_top : ff_bm_max_top;
            ff_bm_max_spanline_dist = (curr_spanline_dist > ff_bm_max_spanline_dist) ? curr_spanline_dist : ff_bm_max_spanline_dist;

            auto glyph_data = std::span<uint8_t> { ff->glyph->bitmap.buffer, ff->glyph->bitmap.width * ff->glyph->bitmap.rows };

            ff_glyphs.emplace_back(
                FreeTypeGlyph {
                    .buffer = { glyph_data.begin(), glyph_data.end() },
                    .width = static_cast<int>(ff->glyph->bitmap.width),
                    .height = static_cast<int>(ff->glyph->bitmap.rows),
                    .left = ff->glyph->bitmap_left,
                    .top = ff->glyph->bitmap_top });
        }

        const size_t internal_bm_w = ff_bm_max_width;
        const size_t internal_bm_h = ff_bm_max_spanline_dist + ff_bm_max_top;
        const size_t internal_spanline = ff_bm_max_spanline_dist;

        std::vector<uint8_t> img;
        img.resize(internal_bm_w * internal_bm_h * FontAtlasConstants::DRAWABLE_CHARS.size());
        std::ranges::fill(img, (uint8_t)0);
#if 1
        for (const auto [i, c] : FontAtlasConstants::DRAWABLE_CHARS | std::views::enumerate) {
#else
        auto& dc = FontAtlasConstants::DRAWABLE_CHARS;
        std::ptrdiff_t i = 0;
        for (auto iter = dc.begin(); iter != dc.end(); ++iter, ++i) {
#endif
            const auto& ff_glyph = ff_glyphs[i];

            std::ptrdiff_t bitmap_offset = (internal_bm_w * internal_bm_h * i);
            uint8_t* bitmap = img.data() + bitmap_offset;
            assert(bitmap_offset < img.size());

            for (int y = 0; y < ff_glyph.height; ++y) {
                for (int x = 0; x < ff_glyph.width; ++x) {
                    const uint8_t& val = *(ff_glyph.buffer.data() + (y * ff_glyph.width + x));
                    uint8_t& dest = *(bitmap + (internal_bm_w * (internal_spanline - ff_glyph.top + y)) + x + ff_glyph.left);
                    dest = val;
                }
            }
        }

#if 0
        for (size_t y = 0; y < internal_bm_h * FontAtlasConstants::DRAWABLE_CHARS.size(); ++y) {
            for (size_t x = 0; x < internal_bm_w; ++x) {
                if (y == internal_spanline) {
                    std::cout << '-';
                } else {
                    auto res = static_cast<int>(std::clamp(*(img.data() + (y * internal_bm_w + x)), (uint8_t)0, (uint8_t)9));
                    if (res == 0) {
                        std::cout << ' ';
                    } else {
                        std::cout << res;
                    }
                }
            }
            std::cout << "|\n|";
        }
#endif
        Media::Image image;
        auto result = image.init_raw(
            std::move(img),
            internal_bm_w,
            internal_bm_h * FontAtlasConstants::DRAWABLE_CHARS.size(),
            Media::ColourFormat::greyscale);
        if (result.has_error()) {
            return result.error();
        }
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