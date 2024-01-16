#pragma once

#include <cstdint>
#include <optional>


#include <Utily/Utily.hpp>

namespace Renderer {

    class VertexArray
    {
    public:
        VertexArray() = default;
        VertexArray(const VertexArray&) = delete;
        VertexArray(VertexArray&& other);

        auto init() noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

        void bind() noexcept;
        void unbind() noexcept;

    private:
        std::optional<uint32_t> _id;
    };
}