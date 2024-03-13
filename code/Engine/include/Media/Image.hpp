#pragma once

#include "../Config.hpp"

#include <Utily/Utily.hpp>
#include <cassert>
#include <cstdint>
#include <optional>

#include <glm/vec2.hpp>

namespace Media {
    class Image
    {
    public:
        enum class InternalFormat {
            undefined = 0,
            greyscale,
            rgba
        };

        [[nodiscard]] static auto create(std::filesystem::path path) -> Utily::Result<Image, Utily::Error>;
        [[nodiscard]] static auto create(std::span<const uint8_t> raw_bytes, glm::uvec2 dimensions, InternalFormat format) -> Utily::Result<Image, Utily::Error>;
        [[nodiscard]] static auto create(std::unique_ptr<uint8_t[]>&& data, size_t data_size_bytes, glm::uvec2 dimensions, InternalFormat format) -> Utily::Result<Image, Utily::Error>;

        [[nodiscard]] inline auto raw_bytes() const noexcept { return std::span { _m.data.get(), _m.data_size_bytes }; }
        [[nodsicard]] inline auto dimensions() const noexcept { return _m.dimensions; }
        [[nodiscard]] inline auto format() const { return _m.format; }

        [[nodiscard]] auto opengl_format() const -> uint32_t;
        [[nodiscard]] auto libspng_format() const -> uint8_t;

        [[nodiscard]] auto save_to_disk(std::filesystem::path path) const noexcept -> Utily::Result<void, Utily::Error>;

        Image(Image&& other);
        Image(const Image&) = delete;

    private:
        struct M {
            std::unique_ptr<uint8_t[]> data = {};
            size_t data_size_bytes = 0;
            glm::uvec2 dimensions = { 0, 0 };
            InternalFormat format = InternalFormat::undefined;
        } _m;

        explicit Image(M&& m)
            : _m(std::move(m)) { }
    };
}