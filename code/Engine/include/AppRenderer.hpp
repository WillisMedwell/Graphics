#pragma once

#include <Utily/Utily.hpp>

#include "Renderer/Renderer.hpp"

class AppRenderer {
    static Utily::StaticVector<Renderer::Shader, 20> shaders;
    static Utily::StaticVector<Renderer::VertexBuffer, 20> vertex_buffers;
    static Utily::StaticVector<Renderer::IndexBuffer, 20> index_buffers;

public:
    float window_width;
    float window_height;

    [[nodiscard]] auto add_shader(std::string_view vertex, std::string_view fragment) noexcept -> Utily::Result<int, Utily::Error>;
    [[nodiscard]] auto get_shader(int shader_id) noexcept -> Renderer::Shader&;

    void stop() noexcept;
};