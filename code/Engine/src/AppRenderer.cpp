#include "AppRenderer.hpp"

Utily::StaticVector<Renderer::Shader, 50> AppRenderer::shaders;

auto AppRenderer::add_shader(std::string_view vertex, std::string_view fragment) noexcept -> Utily::Result<int, Utily::Error> {
    int id = shaders.size();
    shaders.emplace_back();
    auto result = shaders[id].init(vertex, fragment);

    if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::NONE) {
        if (result.has_error()) {
            return Utily::Error { 
                std::format("Shader {} failed. {}", id, result.error().what()) 
            };
        }
    }
    return id;
}

auto AppRenderer::get_shader(int shader_id) noexcept -> Renderer::Shader& {
    if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::NONE) {
        std::cerr << "Shader Id out of bounds.";
        // not really meant to do this inside noexcept but idc.
        assert(false);
    }
    return shaders[shader_id];
}