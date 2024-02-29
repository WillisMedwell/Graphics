#pragma once

#include <Utily/Utily.hpp>

#include "Config.hpp"
#include "Model/Types.hpp"
#include "Profiler/Profiler.hpp"
#include "Core/DebugOpRecorder.hpp"


namespace Core {
    class IndexBuffer
    {
    public:
        IndexBuffer() = default;
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer(IndexBuffer&& other) noexcept;

        [[nodiscard]] auto init() noexcept -> Utily::Result<void, Utily::Error>;
        void stop() noexcept;

        void bind() noexcept;
        void unbind() noexcept;

        template <typename Range>
            requires std::ranges::range<Range>
            && std::contiguous_iterator<std::ranges::iterator_t<Range>>
            && std::same_as<std::ranges::range_value_t<Range>, uint32_t>
            && std::ranges::sized_range<Range>
        void load_indices(const Range& indices) noexcept {
            Core::DebugOpRecorder::instance().push("Core::IndexBuffer", "load_indices()");
            Profiler::Timer timer("Core::IndexBuffer::load_indices", { "rendering" });
            
            this->bind();
            size_t size_in_bytes = indices.size() * sizeof(Model::Index);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, &(*indices.begin()), GL_DYNAMIC_DRAW);
            _count = indices.size();
        }

        size_t get_count() const noexcept { return _count; }

    private:
        std::optional<uint32_t> _id = std::nullopt;
        size_t _count;
    };
}