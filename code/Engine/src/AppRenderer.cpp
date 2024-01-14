#include "AppRenderer.hpp"

Utily::StaticVector<Renderer::Shader, 50> AppRenderer::shaders;

auto AppRenderer::add_shader(std::string_view vertex, std::string_view fragment) noexcept -> Utily::Result<int, Utily::Error> {
    int id = shaders.size();
    shaders.emplace_back(vertex, fragment);
    if (auto result = shaders[id].init(); result.has_error()) {
        return Utily::Error { result.what() };
    }
}

std::optional<std::string&>

    auto AppRenderer::find_shader(const std::string& alias) noexcept -> Utily::Result<Renderer::Shader*, Utily::Error> {
    if (!shaders.contains(alias)) {
    }
}