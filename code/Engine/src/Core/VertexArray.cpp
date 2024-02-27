#include "Core/VertexArray.hpp"

#include "Config.hpp"

namespace Core {
    constexpr static uint32_t INVALID_ARRAY_OBJECT_ID = 0;
    static VertexArray* last_bound_va = nullptr;

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : _id(std::exchange(other._id, std::nullopt)) {
        last_bound_va = nullptr;
    }

    auto VertexArray::init() noexcept -> Utily::Result<void, Utily::Error> {
        if (_id) {
            return Utily::Error { "Trying to override in-use Vertex Array Object" };
        }
        _id = INVALID_ARRAY_OBJECT_ID;
        glGenVertexArrays(1, &_id.value());
        if (_id.value() == INVALID_ARRAY_OBJECT_ID) {
            _id = std::nullopt;
            return Utily::Error { "Failed to create Vertex Array Object. glGenVertexArrays failed." };
        }
        return {};
    }

    void VertexArray::stop() noexcept {
        if (_id.value_or(INVALID_ARRAY_OBJECT_ID) == INVALID_ARRAY_OBJECT_ID) {
            glDeleteVertexArrays(1, &_id.value());
        }
        _id = std::nullopt;
        if (last_bound_va == this) {
            last_bound_va = nullptr;
        }
    }

    void VertexArray::bind() noexcept {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ARRAY_OBJECT_ID) == INVALID_ARRAY_OBJECT_ID) {
                std::cerr << "Trying to bind invalid vertex array object.";
                assert(false);
            }
        }
        if (last_bound_va != this) {
            glBindVertexArray(_id.value_or(INVALID_ARRAY_OBJECT_ID));
            last_bound_va = this;
        }
    }
    void VertexArray::unbind() noexcept {
        if constexpr (Config::SKIP_UNBINDING) {
            return;
        } else if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ARRAY_OBJECT_ID) == INVALID_ARRAY_OBJECT_ID) {
                std::cerr << "Trying to unbind invalid vertex array object.";
                assert(false);
            }
        }

        if (last_bound_va != nullptr) {
            glBindVertexArray(0);
            last_bound_va = nullptr;
        }
    }

}