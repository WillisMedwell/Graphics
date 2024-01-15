#pragma once

#include <Utily/Utily.hpp>

#include "Renderer/Shader.hpp"

class AppRenderer {
    static Utily::StaticVector<Renderer::Shader, 50> shaders;
public:
    float window_width;
    float window_height;

    [[nodiscard]] auto add_shader(std::string_view vertex, std::string_view fragment) noexcept -> Utily::Result<int, Utily::Error>;
    [[nodiscard]] auto get_shader(int shader_id) noexcept -> Renderer::Shader&;
};