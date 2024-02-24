#pragma once

#include "Config.hpp"
#include "Renderer/Fence.hpp"

#include <Utily/Utily.hpp>
#include <cassert>
#include <cstdint>
#include <optional>

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

        [[nodiscard]] auto data() noexcept -> std::optional<std::tuple<std::span<const uint8_t>, uint32_t, uint32_t, ColourFormat>>;

        void add_fence(Renderer::Fence&& fence) noexcept;
        ~Image();

        [[nodiscard]] inline auto width() const noexcept { return _width; }
        [[nodiscard]] inline auto height() const noexcept { return _height; }


    private:
        std::vector<uint8_t> _data;
        Media::ColourFormat _format = Media::ColourFormat::rgb;
        uint32_t _width { 0 }, _height { 0 };
        std::optional<Renderer::Fence> _fence;
    };
}