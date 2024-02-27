#include "Core/IndexBuffer.hpp"

#include <utility>

namespace Core {
    constexpr static uint32_t INVALID_INDEX_BUFFER_ID = 0;
    static IndexBuffer* last_bound_ib = nullptr;

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt))
        , _count(std::exchange(other._count, 0)) {
        last_bound_ib = nullptr;
    }

    auto IndexBuffer::init() noexcept -> Utily::Result<void, Utily::Error> {
        if (_id.has_value()) {
            return Utily::Error { "Trying to override in-use index buffer" };
        }
        _id = INVALID_INDEX_BUFFER_ID;
        _count = 0;
        glGenBuffers(1, &_id.value());
        if (_id.value() == INVALID_INDEX_BUFFER_ID) {
            _id = std::nullopt;
            return Utily::Error { "Failed to create Index Buffer. glGenBuffers failed." };
        }
        return {};
    }

    void IndexBuffer::stop() noexcept {
        if (_id.value_or(INVALID_INDEX_BUFFER_ID) != INVALID_INDEX_BUFFER_ID) {
            glDeleteBuffers(1, &_id.value());
        }
        _id = std::nullopt;
        _count = 0;
        if (last_bound_ib == this) {
            last_bound_ib = nullptr;
        }
    }

    void IndexBuffer::bind() noexcept {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_INDEX_BUFFER_ID) == INVALID_INDEX_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid vertex buffer.";
                assert(false);
            }
        }
        if (last_bound_ib != this) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id.value_or(INVALID_INDEX_BUFFER_ID));
            last_bound_ib = this;
        }
    }

    void IndexBuffer::unbind() noexcept {
        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_INDEX_BUFFER_ID) == INVALID_INDEX_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid vertex buffer.";
                assert(false);
            }
        }

        if (last_bound_ib != nullptr) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            last_bound_ib = nullptr;
        }
    }

}