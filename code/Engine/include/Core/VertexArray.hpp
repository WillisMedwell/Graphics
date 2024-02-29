#pragma once

#include <cstdint>
#include <optional>

#include "Config.hpp"
#include "Core/IndexBuffer.hpp"
#include "Core/VertexBuffer.hpp"
#include "Core/VertexBufferLayout.hpp"
#include "Core/DebugOpRecorder.hpp"
#include <Utily/Utily.hpp>

namespace Core {

    class VertexArray
    {
        constexpr static uint32_t INVALID_ARRAY_OBJECT_ID = 0;

    public:
        VertexArray() = default;
        VertexArray(const VertexArray&) = delete;
        VertexArray(VertexArray&& other) noexcept;

        template <typename... Args>
        [[nodiscard]] auto init(Core::VertexBufferLayout<Args...> vbl, Core::VertexBuffer& vb, Core::IndexBuffer& ib) noexcept -> Utily::Result<void, Utily::Error> {
            Core::DebugOpRecorder::instance().push("Core::VertexArray", "init()");
            
            // generation
            if (_id) {
                return Utily::Error { "Trying to override in-use Vertex Array Object" };
            }
            _id = INVALID_ARRAY_OBJECT_ID;
            glGenVertexArrays(1, &_id.value());
            if (_id.value() == INVALID_ARRAY_OBJECT_ID) {
                _id = std::nullopt;
                return Utily::Error { "Failed to create Vertex Array Object. glGenVertexArrays failed." };
            }
            // bind associated ib, and vb


            ib.bind();
            vb.bind();
            this->bind();

            constexpr static auto layout = vbl.get_layout();
            constexpr static auto stride = vbl.get_stride();

            uint32_t offset = 0;
            for (size_t i = 0; i < layout.size(); i++) {
                const auto& element = layout[i];
#if defined(CONFIG_TARGET_NATIVE)
                glEnableVertexArrayAttrib(get_id().value(), i);
#elif defined(CONFIG_TARGET_WEB)
                glEnableVertexAttribArray(i);
#endif
                glVertexAttribPointer(i, element.count, element.type, element.normalised, stride, reinterpret_cast<const void*>(offset));
                offset += element.type_size;
            }
            return {};
        }

        void stop() noexcept;

        void bind() noexcept;
        void unbind() noexcept;

        auto get_id() const noexcept { return _id; }

    private:
        std::optional<uint32_t> _id;
    };
}