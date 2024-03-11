#pragma once

#include "../Config.hpp"
#include "Core/Fence.hpp"

#include <Utily/Utily.hpp>
#include <cassert>
#include <cstdint>
#include <optional>

#include <glm/vec2.hpp>

#if 0
namespace Media {

    enum class ColourFormat : uint32_t {
        greyscale = GL_R8,
        rgb = GL_RGB8,
        rgba = GL_RGBA8,
        s_rgb = GL_SRGB8,
        s_rgba = GL_SRGB8_ALPHA8,
    };

    class Image
    {
    public:
        Image() = default;
        Image(const Image&) = delete;
        Image(Image&& other);

        Image& operator=(Image&& other) noexcept;

        [[nodiscard]] auto init(
            std::vector<uint8_t>& encoded_png,
            bool include_alpha_channel,
            bool is_gamma_corrected) noexcept
            -> Utily::Result<void, Utily::Error>;

        void init_raw(std::vector<uint8_t>&& raw_data, uint32_t width, uint32_t height, ColourFormat format) noexcept;

        [[nodiscard]] auto save_to_disk(std::filesystem::path path) noexcept
            -> Utily::Result<void, Utily::Error>;

        void stop() noexcept;

        void resize(float scale) noexcept;
        void resize(uint32_t width, uint32_t height) noexcept;

        [[nodiscard]] inline auto raw_bytes() const noexcept -> std::span<const uint8_t> {
            return std::span { _data.cbegin(), _data.cend() };
        }
        [[nodiscard]] inline auto dimensions() const noexcept -> glm::uvec2 { return { _width, _height }; }
        [[nodiscard]] inline auto format() const noexcept -> ColourFormat { return _format; }

        void add_fence(Core::Fence&& fence) noexcept;
        ~Image();

    private:
        std::vector<uint8_t> _data;
        Media::ColourFormat _format = Media::ColourFormat::rgb;
        uint32_t _width { 0 }, _height { 0 };
        std::optional<Core::Fence> _fence;
    };
}
#endif

namespace Media {
    class Image
    {
    public:
        enum class InternalFormat {
            undefined = 0,
            greyscale,
            rgba
        };

        /// @brief Load png image from disk and decode it. Can fail.
        [[nodiscard]] static auto create(std::filesystem::path path)
            -> Utily::Result<Image, Utily::Error>;

        /// @brief Take decoded-raw image data and copy it. Can fail.
        [[nodiscard]] static auto create(std::span<const uint8_t> raw_bytes, glm::uvec2 dimensions, InternalFormat format)
            -> Utily::Result<Image, Utily::Error>;

        /// @brief Take ownership of decoded-raw image data. Can fail.
        [[nodiscard]] static auto create(std::unique_ptr<uint8_t[]>&& data, size_t data_size_bytes, glm::uvec2 dimensions, InternalFormat format)
            -> Utily::Result<Image, Utily::Error>;

        [[nodiscard]] inline auto raw_bytes() const noexcept { return std::span { _m.data.get(), _m.data_size_bytes }; }
        [[nodsicard]] inline auto dimensions() const noexcept { return _m.dimensions; }
        [[nodiscard]] auto format() const { return _m.format; }
        [[nodiscard]] auto opengl_format() const -> uint32_t;

        Image(Image&& other);
        Image(const Image&) = delete;

        auto save_to_disk(std::filesystem::path path) const noexcept -> Utily::Result<void, Utily::Error>;

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