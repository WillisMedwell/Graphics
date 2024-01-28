#pragma once

#include <cstdint>
#include <optional>
#include <ranges>
#include <type_traits>

#include <Utily/Utily.hpp>

#include "Config.hpp"

namespace Renderer {
    class VertexBuffer
    {
    public:
        VertexBuffer() = default;
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer(VertexBuffer&&) noexcept;

        [[nodiscard]] auto init() noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

        void bind() noexcept;
        void unbind() noexcept;

        ~VertexBuffer();

        template <typename Range>
            requires std::ranges::range<Range>
            && std::contiguous_iterator<std::ranges::iterator_t<Range>>
            && std::same_as<std::ranges::range_value_t<Range>, float>
            && std::ranges::sized_range<Range>
        void load_vertices(const Range& vertices) noexcept {
            this->bind();
            size_t size_in_bytes = vertices.size() * sizeof(float);
            glBufferData(GL_ARRAY_BUFFER, size_in_bytes, &(*vertices.begin()), GL_DYNAMIC_DRAW);
        }

    private:
        std::optional<uint32_t> _id = std::nullopt;
    };
}