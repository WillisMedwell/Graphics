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
            && std::ranges::sized_range<Range>
        void load_vertices(const Range& vertices) noexcept {
            this->bind();
            using Underlying = std::ranges::range_value_t<Range>;
            size_t size_in_bytes = vertices.size() * sizeof(Underlying);

#if defined(CONFIG_TARGET_NATIVE)
            glBufferData(GL_ARRAY_BUFFER, size_in_bytes, &(*vertices.begin()), GL_DYNAMIC_DRAW);
#elif defined(CONFIG_TARGET_WEB)
            glBufferData(GL_ARRAY_BUFFER, size_in_bytes, &(*vertices.begin()), GL_STATIC_DRAW);
#endif
        }

    private:
        std::optional<uint32_t> _id = std::nullopt;
    };
}