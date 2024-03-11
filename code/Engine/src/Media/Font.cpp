#include "Media/Font.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <ranges>

#include "Profiler/Profiler.hpp"

// #include <mdspan>

#include <ft2build.h>
#include FT_FREETYPE_H

struct FreeType {
    FT_Library library = nullptr;

    FreeType() {
        Profiler::Timer timer("Font::FreeType() *library init*", { "freetype", "font", "init" });
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

#if 0
namespace Media {
    auto FontAtlas::operator=(FontAtlas&& other) noexcept -> FontAtlas& {
        this->image = std::move(other.image);
        this->columns = other.columns;
        this->rows = other.rows;
        this->glyph_width = other.glyph_width;
        this->glyph_height = other.glyph_height;
        return *this;
    }

    auto FontAtlas::init(Media::Font& font, uint32_t char_height_px) -> Utily::Result<void, Utily::Error> {
        auto res = font.gen_image_atlas(char_height_px);
        if (res.has_error()) {
            return res.error();
        }
        *this = std::move(res.value());

        return {};
    }

    auto FontAtlas::uv_coord_of_char(char a) const -> FontAtlas::UvCoord {
        constexpr auto drawble_chars = FontAtlasConstants::DRAWABLE_CHARS;

        assert(drawble_chars.front() <= a && a <= drawble_chars.back() && "Must be a printable character");

        const auto i = static_cast<int>(a - drawble_chars.front());

        const float r = static_cast<float>(i / columns);
        const float c = static_cast<float>(i % columns);

        return UvCoord {
            .min_x = c / static_cast<float>(columns),
            .max_x = (c + 1) / static_cast<float>(columns),
            .min_y = 1 - (r + 1) / static_cast<float>(rows),
            .max_y = 1 - r / static_cast<float>(rows),
        };
    }

    auto Font::init(std::vector<uint8_t>& encoded_ttf) noexcept -> Utily::Result<void, Utily::Error> {
        FT_Face ff = nullptr;
        if (auto error = FT_New_Memory_Face(free_type.library, encoded_ttf.data(), encoded_ttf.size(), 0, &ff); error) {
            return Utily::Error { FT_Error_String(error) };
        }
        _font_face = ff;
        return {};
    }

    struct FreeTypeGlyph {
        std::vector<uint8_t> buffer = {};
        std::ptrdiff_t width { 0 }, height { 0 }, spanline { 0 }, left_padding { 0 };

        constexpr FreeTypeGlyph() = default;

        constexpr FreeTypeGlyph(const FT_GlyphSlot& slot)
            : width(slot->bitmap.width)
            , height(slot->bitmap.rows)
            , spanline(slot->bitmap_top)
            , left_padding(slot->bitmap_left) {
            auto buffer_data = std::span<uint8_t>(slot->bitmap.buffer, slot->bitmap.width * slot->bitmap.rows);
            buffer = std::vector<uint8_t> { buffer_data.begin(), buffer_data.end() };
        }
    };

    struct GlyphInfo {
        std::ptrdiff_t width = 0, height = 0, spanline = 0, left_padding = 0;

        GlyphInfo() = default;

        GlyphInfo(const FT_GlyphSlot& slot)
            : width(slot->bitmap.width)
            , height(slot->bitmap.rows + 1)
            , spanline(slot->bitmap_top)
            , left_padding(slot->bitmap_left) { }

        static auto take_max_values(const GlyphInfo& lhs, const GlyphInfo& rhs) -> GlyphInfo {
            GlyphInfo gi;
            gi.width = std::max(lhs.width, rhs.width);
            gi.height = std::max(lhs.height, rhs.height);
            gi.spanline = std::max(lhs.spanline, rhs.spanline);
            gi.left_padding = std::max(lhs.left_padding, rhs.left_padding);
            return gi;
        }
    };

    auto Font::gen_image_atlas(uint32_t char_height_px) -> Utily::Result<FontAtlas, Utily::Error> {
        Profiler::Timer timer("Font::gen_image_atlas()", { "font", "loading" });

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
        GlyphInfo atlas_info = {};
        {
            Profiler::Timer timer("FT_Render_glyphs()");
            // 1. Transform -> Generates and caches the freetype bitmaps & returns its glyph layout
            // 2. Reduce    -> Calculate the maximum bounding glyph layout, so all characters have enough space in atlas.
            atlas_info = std::transform_reduce(
                drawable_chars.begin(),
                drawable_chars.end(),
                cached_ft_bitmaps.begin(),
                GlyphInfo {},
                &GlyphInfo::take_max_values,
                generate_and_cache_ft_glyph);
        }
        Profiler::Timer blit_timer("blit_glyphs_into_altas()");

        const auto [atlas_glyphs_per_row, atlas_num_rows] = [](float glyph_width, float glyph_height, float num_glyphs_in_atlas) {

#if 1 // generate most square shape by brute force.
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

        std::vector<uint8_t> atlas_buffer;

        atlas_buffer.resize(atlas_img_height * atlas_img_width, (uint8_t)0);

        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(drawable_chars.size()); ++i) {
            const auto& bitmap_ft = cached_ft_bitmaps[i];
            const auto atlas_coords = glm::ivec2 { i % atlas_glyphs_per_row, i / atlas_glyphs_per_row };
            const auto adjusted_offset = glm::ivec2 { atlas_coords.x * atlas_info.width, atlas_coords.y * atlas_info.height };

            auto get_atlas_buffer_dest = [&](int x, int y) -> uint8_t& {
                const auto px_coords = glm::ivec2(adjusted_offset.x + x, adjusted_offset.y + y);
                return atlas_buffer[px_coords.y * atlas_img_width + px_coords.x + bitmap_ft.left_padding];
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

        FontAtlas font_atlas;
        font_atlas.columns = atlas_glyphs_per_row;
        font_atlas.rows = atlas_num_rows;
        font_atlas.glyph_width = static_cast<int>(atlas_info.width);
        font_atlas.glyph_height = static_cast<int>(atlas_info.height);
        font_atlas.image.init_raw(std::move(atlas_buffer), atlas_img_width, atlas_img_height, ColourFormat::greyscale);
        return font_atlas;
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

    auto FontMeshGenerator::generate_static_mesh(std::string_view str, const float char_height, const glm::vec2 bottom_left_pos, const FontAtlas& atlas) -> std::tuple<std::array<Model::Vertex2D, 400>, std::array<Model::Index, 600>> {
        Profiler::Timer timer("FontMeshGenerate::generate_static_mesh()");

        auto is_not_printable = [](char c) {
            return !(FontAtlasConstants::DRAWABLE_CHARS.front() <= c && c <= FontAtlasConstants::DRAWABLE_CHARS.front());
        };
        assert(std::any_of(str.begin(), str.end(), is_not_printable));

        constexpr int max_chars = 100;
        if (str.size() > max_chars) {
            throw std::length_error("Exceeded maximum char capacity");
        }

        static std::array<Model::Vertex2D, max_chars* 4> vertices = {};
        static std::array<Model::Index, max_chars* 6> indices = {};

        const size_t vert_size = str.size() * 4;
        const size_t indi_size = str.size() * 6;

        Model::Vertex2D* v_ptr[] = {
            vertices.data() + 0,
            vertices.data() + 1,
            vertices.data() + 2,
            vertices.data() + 3
        };
        Model::Index* i_ptr[] = {
            indices.data() + 0,
            indices.data() + 1,
            indices.data() + 2,
            indices.data() + 3,
            indices.data() + 4,
            indices.data() + 5,
        };

        const float char_width = static_cast<float>(atlas.glyph_width) / static_cast<float>(atlas.glyph_height) * char_height;
        const float y_min = bottom_left_pos.y;
        const float y_max = y_min + char_height;

#if 1
        for (int i = 0; i < str.size(); ++i) {
            char c = str[i];
#else
        for (auto [i, c] : str | std::views::enumerate) {
#endif
            const auto uv = atlas.uv_coord_of_char(c);

            const int v_offset = i * 4;
            const int i_offset = i * 6;

            const float x_min = char_width * i + bottom_left_pos.x;
            const float x_max = x_min + char_width;

            std::construct_at(v_ptr[0] + v_offset, glm::vec2(x_min, y_min), glm::vec2(uv.min_x, uv.min_y));
            std::construct_at(v_ptr[1] + v_offset, glm::vec2(x_max, y_min), glm::vec2(uv.max_x, uv.min_y));
            std::construct_at(v_ptr[2] + v_offset, glm::vec2(x_max, y_max), glm::vec2(uv.max_x, uv.max_y));
            std::construct_at(v_ptr[3] + v_offset, glm::vec2(x_min, y_max), glm::vec2(uv.min_x, uv.max_y));

            *(i_ptr[0] + i_offset) = v_offset + 0;
            *(i_ptr[1] + i_offset) = v_offset + 1;
            *(i_ptr[2] + i_offset) = v_offset + 2;
            *(i_ptr[3] + i_offset) = v_offset + 2;
            *(i_ptr[4] + i_offset) = v_offset + 3;
            *(i_ptr[5] + i_offset) = v_offset + 0;
        }
        return std::tuple { vertices, indices };
    }
}
#else

namespace Media {
    constexpr static auto PRINTABLE_CHARS = []() {
        constexpr char first_printable = char(32);
        constexpr char last_printable = char(127);
        constexpr size_t n = last_printable - first_printable;
        std::array<char, n> chars {};
        std::ranges::copy(std::views::iota(first_printable, last_printable), chars.begin());
        return chars;
    }();

    auto FontAtlas::create(std::filesystem::path path, uint32_t char_height_px) noexcept -> Utily::Result<FontAtlas, Utily::Error> {
        // 1. Load ttf file from disk.
        // 2. Initalise the font face
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
        FT_Face ft_face = nullptr;
        if (auto error = FT_New_Memory_Face(free_type.library, encoded_ttf.data(), encoded_ttf.size(), 0, &ft_face); error) {
            return Utily::Error { FT_Error_String(error) };
        }
        if (auto error = FT_Set_Pixel_Sizes(ft_face, 0, char_height_px); error) {
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
        std::array<CachedGlyph, PRINTABLE_CHARS.size()> cached_glyphs;
        std::transform(PRINTABLE_CHARS.begin(), PRINTABLE_CHARS.end(), cached_glyphs.begin(), create_cached_glyph);

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
        }(atlas_info.bitmap_dimensions.x, atlas_info.bitmap_dimensions.y, PRINTABLE_CHARS.size());

        const int atlas_img_height = atlas_info.bitmap_dimensions.y * atlas_num_rows;
        const int atlas_img_width = atlas_info.bitmap_dimensions.x * atlas_glyphs_per_row;

        std::vector<uint8_t> atlas_buffer;
        atlas_buffer.resize(atlas_img_height * atlas_img_width, (uint8_t)0);

        std::cout << atlas_info.bitmap_dimensions.x << ',';
        std::cout << atlas_info.bitmap_dimensions.y << '\n';

        std::cout << atlas_img_width << ',';
        std::cout << atlas_img_height << '\n';

        // 6.
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(PRINTABLE_CHARS.size()); ++i) {
            const auto& bitmap_ft = cached_glyphs[i];
            const auto atlas_coords = glm::ivec2 { i % atlas_glyphs_per_row, i / atlas_glyphs_per_row };
            const auto adjusted_offset = glm::ivec2 { atlas_coords.x * atlas_info.bitmap_dimensions.x, atlas_coords.y * atlas_info.bitmap_dimensions.y };

            auto get_atlas_buffer_dest = [&](int x, int y) -> uint8_t& {
                static int max_y = 0;
                if (y > max_y) {
                    max_y = y;
                    std::cout << "MAX Y -> " << y << '\n';
                }
                const auto px_coords = glm::ivec2(adjusted_offset.x + x, adjusted_offset.y + y);
                return atlas_buffer[px_coords.y * atlas_img_width + px_coords.x + bitmap_ft.dimensions.left_padding];
            };
            for (std::ptrdiff_t y = 0; y < bitmap_ft.dimensions.bitmap_dimensions.y; ++y) {
                for (std::ptrdiff_t x = 0; x < bitmap_ft.dimensions.bitmap_dimensions.x; ++x) {
                    // align to relative bitmap
                    const std::ptrdiff_t relative_y = (y + atlas_info.spanline - bitmap_ft.dimensions.spanline);
                    const std::ptrdiff_t ft_offset = y * bitmap_ft.dimensions.bitmap_dimensions.x + x;
                    get_atlas_buffer_dest(x, relative_y) = bitmap_ft.bitmap[ft_offset];
                }
            }
        }

        // 7.
        auto image_result = Media::Image::create(
            std::span(atlas_buffer.cbegin(), atlas_buffer.cend()),
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
                atlas_info.bitmap_dimensions.y } });
    }
    auto FontAtlas::uv_for(char a) const noexcept -> FontAtlas::UvCoord {
        assert(PRINTABLE_CHARS.front() <= a && a <= PRINTABLE_CHARS.back() && "Must be a printable character");

        const auto i = static_cast<int>(a - PRINTABLE_CHARS.front());
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

#endif