#pragma once

#include <cstdint>
#include <optional>

#include <Utily/Utily.hpp>

#include "Config.hpp"

namespace Renderer {
    class VertexBuffer
    {
    public:
        VertexBuffer() = default;
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer(VertexBuffer&&) noexcept;


        auto init() noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

        void bind() noexcept;
        void unbind() noexcept;

        ~VertexBuffer();

    private:
        std::optional<uint32_t> _id = std::nullopt;
    };
}