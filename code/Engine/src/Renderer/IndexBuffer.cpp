#include "Renderer/IndexBuffer.hpp"

#include <utility>

namespace Renderer {
    constexpr static uint32_t INVALID_BUFFER_ID = 0;

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt))
        , _count(std::exchange(other._count, 0)) { }

    auto IndexBuffer::init() noexcept -> Utily::Result<void, Utily::Error> {
        if (_id.has_value()) {
            return Utily::Error { "Trying to override in-use index buffer" };
        }
        _id = INVALID_BUFFER_ID;
        _count = 0;
        glGenBuffers(1, &_id.value());
        if (_id.value() == INVALID_BUFFER_ID) {
            _id = std::nullopt;
            return Utily::Error { "Failed to create Index Buffer. glGenBuffers failed." };
        }
        return {};
    }
    
    static IndexBuffer* last_bound = nullptr;
    
    void IndexBuffer::stop() noexcept {
        if (_id.value_or(INVALID_BUFFER_ID) != INVALID_BUFFER_ID) {
            glDeleteBuffers(1, &_id.value());
        }
        _id = std::nullopt;
        _count = 0;
        if(last_bound == this) {
            last_bound = nullptr;
        }
    }


    void IndexBuffer::bind() noexcept {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_BUFFER_ID) == INVALID_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid vertex buffer.";
                assert(false);
            }
        }
        if(last_bound != this) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id.value_or(INVALID_BUFFER_ID));
            last_bound = this;
        }
    }

    void IndexBuffer::unbind() noexcept {
        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_BUFFER_ID) == INVALID_BUFFER_ID) {
                std::cerr << "Trying to unbind invalid vertex buffer.";
                assert(false);
            }
        }

        if (last_bound != nullptr) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            last_bound = nullptr;
        }
    }

}