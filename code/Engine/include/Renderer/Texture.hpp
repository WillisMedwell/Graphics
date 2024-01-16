#pragma once

#include <Utily/Utily.hpp>

#include "Config.hpp"

namespace Renderer {

    class Texture
    {
    public:
        enum class Filter {
            smooth = GL_LINEAR,
            pixelated = GL_NEAREST
        };

        enum class ColourCorrection {
            corrected = GL_SRGB,
            uncorrected = GL_RGB
        };

        enum class BindType : uint32_t {
            multisample,
            basic = GL_TEXTURE_2D
        };

        Texture() = default;
        Texture(const Texture&) = delete;
        Texture(Texture&&);

        auto init() noexcept -> Utily::Result<void, Utily::Error>;

        void setup_for_framebuffer(
            uint32_t width,
            uint32_t height,
            BindType sample_type = BindType::multisample) noexcept;

        // Once texture unit is aquired, it cannot be taken away unless unbinded() or just bind(false).
        [[nodiscard]] auto bind(bool locked = false) noexcept -> Utily::Result<uint32_t, Utily::Error>;
        void unbind() noexcept;
        void stop() noexcept;
        inline auto unit() const noexcept { return _texture_unit_index; }

        ~Texture();

    private:
        BindType _bind_type = BindType::basic;
        std::optional<uint32_t> _id = std::nullopt;
        std::optional<uint32_t> _texture_unit_index = std::nullopt;
        uint32_t _width { 0 }, _height { 0 };
    };
}