#include "Renderer/VertexBuffer.hpp"

#include <utility>

namespace Renderer {
    constexpr static uint32_t INVALID_VERTEX_BUFFER_ID = 0;
    static VertexBuffer* last_bound_vb = nullptr;

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt)) {
        last_bound_vb = nullptr;
    }

    auto VertexBuffer::init() noexcept -> Utily::Result<void, Utily::Error> {
        if (_id) {
            return Utily::Error { "Trying to override in-use vertex buffer" };
        }
        _id = INVALID_VERTEX_BUFFER_ID;
        glGenBuffers(1, &_id.value());
        if (_id.value() == INVALID_VERTEX_BUFFER_ID) {
            _id = std::nullopt;
            return Utily::Error { "Failed to create Vertex buffer. glGenBuffers failed." };
        }
        return {};
    }

    void VertexBuffer::stop() noexcept {
        if (_id.value_or(INVALID_VERTEX_BUFFER_ID) != INVALID_VERTEX_BUFFER_ID) {
            glDeleteBuffers(1, &_id.value());
        }
        _id = std::nullopt;

        if (last_bound_vb == this) {
            last_bound_vb = this;
        }
    }

    void VertexBuffer::bind() noexcept {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_VERTEX_BUFFER_ID) == INVALID_VERTEX_BUFFER_ID) {
                std::cerr << "Trying to bind invalid vertex buffer.";
                assert(false);
            }
        }
        if (last_bound_vb != this) {
            glBindBuffer(GL_ARRAY_BUFFER, _id.value_or(INVALID_VERTEX_BUFFER_ID));
            last_bound_vb = this;
        }
    }
    void VertexBuffer::unbind() noexcept {
        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_VERTEX_BUFFER_ID) == INVALID_VERTEX_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid vertex buffer.";
                assert(false);
            }
        }

        if (last_bound_vb != nullptr) {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            last_bound_vb = nullptr;
        }
    }

    VertexBuffer::~VertexBuffer() noexcept {
        stop();
    }
}