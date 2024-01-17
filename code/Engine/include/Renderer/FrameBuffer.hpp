#pragma once

#include <Utily/Utily.hpp>

#include "Config.hpp"
#include <optional>

namespace Renderer {
    class FrameBuffer
    {
    public:
        FrameBuffer() = default;
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer(FrameBuffer&& other) noexcept;

        auto init(uint32_t width, uint32_t height) noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

        void bind() noexcept;
        void unbind() noexcept;

        ~FrameBuffer() noexcept;

    private:
        std::optional<uint32_t> _id = std::nullopt;
        std::optional<uint32_t> _colour_attachment_index = std::nullopt;
        uint32_t _width { 0 }, _height { 0 };
    };
}