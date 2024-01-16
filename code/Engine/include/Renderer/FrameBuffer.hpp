#pragma once

#include <Utily/Utily.hpp>

#include "Config.hpp"

namespace Renderer {
    class FrameBuffer
    {
        FrameBuffer() = default;
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer(FrameBuffer&& other);

        auto init() noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

    private:
        
    };
}