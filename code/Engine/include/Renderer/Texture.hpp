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

        Texture() = default;
        Texture(const Texture&) = delete;
        Texture(Texture&&);

        auto init(
            uint32_t width,
            uint32_t height,
            Filter filter = Filter::pixelated,
            ColourCorrection colour_correction = ColourCorrection::uncorrected) noexcept
            -> Utily::Result<void, Utily::Error>;

        auto resize(uint32_t width, uint32_t height) noexcept -> Utily::Result<void, Utily::Error>;
                
        // Once texture unit is aquired, it cannot be taken away unless unbinded() or just binded().
        [[nodsicard]] auto locked_bind() -> Utily::Result<void, Utily::Error>; 
        [[nodsicard]] auto bind() -> Utily::Result<void, Utily::Error>;  
        void unbind();


        // [[nodiscard]] bool is_bound() const noexcept;
        // [[nodiscard]] auto bound_slot() const noexcept -> uint32_t;

    private:
        std::optional<uint32_t> _id = std::nullopt;
        uint32_t _width { 0 }, _height { 0 };
        std::optional<uint32_t> _texture_unit_index;
    };
}