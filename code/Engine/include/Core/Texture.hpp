#pragma once

#include <Utily/Utily.hpp>

#include "Media/Image.hpp"
#include "Config.hpp"

namespace Core {

    class Texture
    {
    public:
        enum class Filter : int32_t {
            smooth = GL_LINEAR,
            pixelated = GL_NEAREST
        };

        Texture() = default;
        Texture(const Texture&) = delete;
        Texture(Texture&&);

        [[nodiscard]] auto init() noexcept -> Utily::Result<void, Utily::Error>;
        auto upload_image(const Media::Image& image, Filter filter = Filter::smooth) noexcept -> Utily::Result<void, Utily::Error>;

        // Once texture unit is aquired, it cannot be taken away unless unbinded() or just bind(false).
        [[nodiscard]] auto bind(bool locked = false) noexcept -> Utily::Result<uint32_t, Utily::Error>;
        void unbind() noexcept;
        void stop() noexcept;
        inline auto unit() const noexcept { return _texture_unit_index; }

        ~Texture();

    private:
        std::optional<uint32_t> _id = std::nullopt;
        std::optional<uint32_t> _texture_unit_index = std::nullopt;
        uint32_t _width { 0 }, _height { 0 };
    };
}