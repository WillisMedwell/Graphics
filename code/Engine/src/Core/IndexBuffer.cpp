#include "Core/IndexBuffer.hpp"

#include <utility>

namespace Core {
    constexpr static uint32_t INVALID_INDEX_BUFFER_ID = 0;

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt))
        , _count(std::exchange(other._count, 0)) {
    }

    auto IndexBuffer::init() noexcept -> Utily::Result<void, Utily::Error> {
        Core::DebugOpRecorder::instance().push("Core::IndexBuffer", "init()");

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
        Core::DebugOpRecorder::instance().push("Core::IndexBuffer", "stop()");

        if (_id.value_or(INVALID_INDEX_BUFFER_ID) != INVALID_INDEX_BUFFER_ID) {
            glDeleteBuffers(1, &_id.value());
        }
        _id = std::nullopt;
        _count = 0;
    }

    void IndexBuffer::bind() noexcept {
        Core::DebugOpRecorder::instance().push("Core::IndexBuffer", "bind()");

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_INDEX_BUFFER_ID) == INVALID_INDEX_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid vertex buffer.";
                assert(false);
            }
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id.value_or(INVALID_INDEX_BUFFER_ID));
    }

    void IndexBuffer::unbind() noexcept {
        Core::DebugOpRecorder::instance().push("Core::IndexBuffer", "unbind()");

        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_INDEX_BUFFER_ID) == INVALID_INDEX_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid vertex buffer.";
                assert(false);
            }
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

}