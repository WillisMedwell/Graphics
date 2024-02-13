#pragma once

#include <Utily/Utily.hpp>

#include "Media/Image.hpp"
#include <array>
#include <limits>
#include <ranges>

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
#if 0
            for (auto [i, e] : table | std::views::enumerate) {
                e = std::ranges::contains(DRAWABLE_CHARS, static_cast<char>(i));
            }
#else
            std::ptrdiff_t i = 0;
            for (auto iter = table.begin(); iter != table.end(); ++iter, ++i) {
                *iter = std::ranges::find(DRAWABLE_CHARS, static_cast<char>(i)) != DRAWABLE_CHARS.end();
            }
#endif
            return table;
        }
        constexpr static auto IS_CHAR_DRAWABLE = gen_is_char_drawable_table();
    }

    class Font
    {
    public:
        Font() = default;
        Font(const Font&) = delete;
        Font(Font&& other);

        [[nodiscard]] auto init(std::vector<uint8_t>& encoded_ttf) noexcept -> Utily::Result<void, Utily::Error>;
        [[nodiscard]] auto gen_image_atlas(uint32_t char_height_px) -> Utily::Result<Media::Image, Utily::Error>;

        void stop() noexcept;

        ~Font();

    private:
        void* _font_face = nullptr;
    };

}