#pragma once

#include <Utily/Utily.hpp>

#include "Config.hpp"

namespace Renderer
{
    class IndexBuffer
    {   
        IndexBuffer() = default;
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer(IndexBuffer&& other) noexcept;

        auto init() noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

        void bind() noexcept;
        void unbind() noexcept; 
    private:
        std::optional<uint32_t> _id = std::nullopt;
        size_t _count;
    };   
}