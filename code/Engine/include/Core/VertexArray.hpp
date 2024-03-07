#pragma once

#include <cstdint>
#include <optional>

#include "Config.hpp"
#include "Core/DebugOpRecorder.hpp"
#include "Core/IndexBuffer.hpp"
#include "Core/VertexBuffer.hpp"
#include "Core/VertexBufferLayout.hpp"
#include <Utily/Utily.hpp>

namespace Core {

    class VertexArray
    {
        constexpr static uint32_t INVALID_ARRAY_OBJECT_ID = 0;

        using VBLMat4 = Core::VertexBufferLayout<glm::vec4, glm::vec4, glm::vec4, glm::vec4>;

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
            this->bind();
            ib.bind();
            vb.bind();

            constexpr static auto layout = vbl.get_layout();
            constexpr static auto stride = vbl.get_stride();

            uint32_t offset = 0;
            for (size_t i = 0; i < layout.size(); i++) {
                const auto& element = layout[i];
                glVertexAttribPointer(i, element.count, element.type, element.normalised, stride, reinterpret_cast<const void*>(offset));
                glEnableVertexAttribArray(i);
                offset += element.type_size;
            }
            return {};
        }

        template <typename... Args>
        [[nodiscard]] auto init(
            Core::VertexBufferLayout<Args...> vbl,
            Core::VertexBuffer& vb_mesh,
            Core::VertexBuffer& vb_transforms,
            Core::IndexBuffer& ib) noexcept -> Utily::Result<void, Utily::Error> {

            Core::DebugOpRecorder::instance().push("Core::VertexArray", "init_as_instance()");
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

            this->bind();
            ib.bind();

            size_t layout_index = 0;
            { // set the VA attribs for the mesh VB
                vb_mesh.bind();

                constexpr static auto layout = vbl.get_layout();
                constexpr static auto stride = vbl.get_stride();
                for (uint32_t offset = 0; layout_index < layout.size(); ++layout_index) {
                    const auto& element = layout[layout_index];
                    glVertexAttribPointer(layout_index, element.count, element.type, element.normalised, stride, reinterpret_cast<const void*>(offset));
                    glEnableVertexAttribArray(layout_index);
                    glVertexAttribDivisor(layout_index, 0);
                    offset += element.type_size;
                }
            }

            { // set the VA attribs for the instance transform VB
                vb_transforms.bind();

                constexpr static auto layout = VBLMat4 {}.get_layout();
                constexpr static auto stride = VBLMat4 {}.get_stride();

                uint32_t max_layout_index = layout_index + layout.size();

                for (uint32_t i = 0, offset = 0; layout_index < max_layout_index; ++i, ++layout_index) {
                    const auto& element = layout[i];
                    glVertexAttribPointer(layout_index, element.count, element.type, element.normalised, stride, reinterpret_cast<const void*>(offset));
                    glEnableVertexAttribArray(layout_index);
                    glVertexAttribDivisor(layout_index, 1);
                    offset += element.type_size;
                }
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