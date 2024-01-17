#pragma once

#include "Config.hpp"
#include "Renderer/Fence.hpp"

#include <Utily/Utily.hpp>
#include <cassert>
#include <cstdint>
#include <optional>

namespace Media {

    enum class ColourFormat : uint32_t {
        rgb = GL_RGB8,
        rgba = GL_RGBA8,
        s_rgb = GL_SRGB8,
        s_rgba = GL_SRGB8_ALPHA8
    };

    class Image
    {
    public:
        Image() = default;
        Image(const Image&) = delete;
        Image(Image&& other);

        [[nodiscard]] auto init(
            std::vector<uint8_t>& encoded_png,
            bool include_alpha_channel,
            bool is_gamma_corrected) noexcept
            -> Utily::Result<void, Utily::Error>;

        void stop() noexcept;

        [[nodiscard]] auto data() noexcept -> std::optional<std::tuple<std::span<const uint8_t>, uint32_t, uint32_t, ColourFormat>>;

        void add_fence(Renderer::Fence&& fence) noexcept;
        ~Image();

    private:
        std::vector<uint8_t> _data;
        Media::ColourFormat _format = Media::ColourFormat::rgb;
        uint32_t _width { 0 }, _height { 0 };
        std::optional<Renderer::Fence> _fence;
    };
}