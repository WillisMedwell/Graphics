#pragma once

#include <Utily/Utily.hpp>

#include "Renderer/Renderer.hpp"

class AppRenderer
{
public:
    // pretty much phantom types.
    struct ShaderId {
        std::ptrdiff_t id;
    };
    struct IndexBufferId {
        std::ptrdiff_t id;
    };
    struct VertexBufferId {
        std::ptrdiff_t id;
    };
    struct VertexArrayId {
        std::ptrdiff_t id;
    };

    Utily::StaticVector<Renderer::Shader, 20> shaders;
    Utily::StaticVector<Renderer::VertexBuffer, 20> vertex_buffers;
    Utily::StaticVector<Renderer::IndexBuffer, 20> index_buffers;
    Utily::StaticVector<Renderer::VertexArray, 20> vertex_arrays;

    float window_width;
    float window_height;

    [[nodiscard]] auto add_shader(std::string_view vertex, std::string_view fragment) noexcept -> Utily::Result<ShaderId, Utily::Error>;
    [[nodiscard]] auto add_vertex_buffer() noexcept -> Utily::Result<VertexBufferId, Utily::Error>;
    [[nodiscard]] auto add_index_buffer() noexcept -> Utily::Result<IndexBufferId, Utily::Error>;

    template <typename... Args>
    [[nodiscard]] auto add_vertex_array(Renderer::VertexBufferLayout<Args...> vertex_buffer_layout, VertexBufferId vb_id) -> Utily::Result<VertexArrayId, Utily::Error> {
        auto id = vertex_arrays.size();
        vertex_arrays.emplace_back();

        Renderer::VertexArray& va = vertex_arrays[id];
        auto result = va.init();
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (result.has_error()) {
                return result.error();
            }
        }

        va.bind();
        vertex_buffers[vb_id.id].bind();

        constexpr static auto layout = vertex_buffer_layout.get_layout();
        constexpr static auto stride = vertex_buffer_layout.get_stride();

        uint32_t offset = 0;
        for (size_t i = 0; i < layout.size(); i++) {
            const auto& element = layout[i];
#if defined(CONFIG_TARGET_NATIVE)
            glEnableVertexArrayAttrib(va.get_id().value(), i);
            glVertexAttribPointer(i, element.count, element.type, element.normalised, stride, reinterpret_cast<const void*>(offset));
#elif defined(CONFIG_TARGET_WEB)
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, element.count, element.type, element.normalised, stride, reinterpret_cast<const void*>(offset));
#endif
            offset += element.count * element.type_size;
        }
        return 0;
    }

    void stop() noexcept;
};