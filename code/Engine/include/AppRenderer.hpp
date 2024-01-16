#pragma once

#include <Utily/Utily.hpp>

#include "Renderer/Renderer.hpp"

class AppRenderer {
public:
    Utily::StaticVector<Renderer::Shader, 20> shaders;
    Utily::StaticVector<Renderer::VertexBuffer, 20> vertex_buffers;
    Utily::StaticVector<Renderer::IndexBuffer, 20> index_buffers;
    Utily::StaticVector<Renderer::VertexArray, 20> vertex_arrays;

    float window_width;
    float window_height;

    [[nodiscard]] auto add_shader(std::string_view vertex, std::string_view fragment) noexcept -> Utily::Result<int, Utily::Error>;
    [[nodiscard]] auto add_vertex_buffer() -> Utily::Result<int, Utily::Error>;
    [[nodiscard]] auto add_index_buffer() -> Utily::Result<int, Utily::Error>;


    template<typename... Args>
    [[nodiscard]] auto add_vertex_array(Renderer::VertexBufferLayout<Args...> vertex_buffer_layout) -> Utily::Result<int, Utily::Error>{
        auto id = vertex_arrays.size();
        vertex_arrays.emplace_back();

        Renderer::VertexArray& va = vertex_arrays[id];
        auto result = va.init();
        if constexpr(Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if(result.has_error()) {
                return result.error();
            }
        }

        return {0};
    }

    void stop() noexcept;
};