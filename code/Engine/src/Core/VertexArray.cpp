#include "Core/VertexArray.hpp"

#include "Config.hpp"

namespace Core {

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt)) {
    }

    void VertexArray::stop() noexcept {
        Core::DebugOpRecorder::instance().push("Core::VertexArray", "stop()");

        if (_id.value_or(INVALID_ARRAY_OBJECT_ID) == INVALID_ARRAY_OBJECT_ID) {
            glDeleteVertexArrays(1, &_id.value());
        }
        _id = std::nullopt;
    }

    void VertexArray::bind() noexcept {
        Core::DebugOpRecorder::instance().push("Core::VertexArray", "bind()");

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ARRAY_OBJECT_ID) == INVALID_ARRAY_OBJECT_ID) {
                std::cerr << "Trying to bind invalid vertex array object.";
                assert(false);
            }
        }

        glBindVertexArray(_id.value_or(INVALID_ARRAY_OBJECT_ID));
    }
    void VertexArray::unbind() noexcept {
        Core::DebugOpRecorder::instance().push("Core::VertexArray", "unbind()");

        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ARRAY_OBJECT_ID) == INVALID_ARRAY_OBJECT_ID) {
                std::cerr << "Trying to unbind invalid vertex array object.";
                assert(false);
            }
        }
        glBindVertexArray(0);
    }

}