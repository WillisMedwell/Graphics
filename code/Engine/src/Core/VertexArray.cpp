#include "Core/VertexArray.hpp"

#include "Config.hpp"

namespace Core {

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt)) {
    }

    // auto VertexArray::init() noexcept -> Utily::Result<void, Utily::Error> {
    //     if (_id) {
    //         return Utily::Error { "Trying to override in-use Vertex Array Object" };
    //     }
    //     _id = INVALID_ARRAY_OBJECT_ID;
    //     glGenVertexArrays(1, &_id.value());
    //     if (_id.value() == INVALID_ARRAY_OBJECT_ID) {
    //         _id = std::nullopt;
    //         return Utily::Error { "Failed to create Vertex Array Object. glGenVertexArrays failed." };
    //     }
    //     return {};
    // }

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