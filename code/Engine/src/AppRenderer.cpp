#include "AppRenderer.hpp"

#include <format>

auto AppRenderer::add_shader(std::string_view vertex, std::string_view fragment) noexcept -> Utily::Result<AppRenderer::ShaderId, Utily::Error> {
    auto id = shaders.size();
    shaders.emplace_back();
    auto result = shaders[id].init(vertex, fragment);

    if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
        if (result.has_error()) {
            return Utily::Error {
                std::format("Shader {} failed. {}", id, result.error().what())
            };
        }
    }
    return id;
}

auto AppRenderer::add_vertex_buffer() noexcept -> Utily::Result<AppRenderer::VertexBufferId, Utily::Error> {
    auto id = vertex_buffers.size();

    vertex_buffers.emplace_back();
    auto result = vertex_buffers[id].init();
    if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
        if (result.has_error()) {
            return Utily::Error {
                std::format("VertexBuffer {} failed. {}", id, result.error().what())
            };
        }
    }
    return id;
}
auto AppRenderer::add_index_buffer() noexcept -> Utily::Result<AppRenderer::IndexBufferId, Utily::Error> {
    auto id = index_buffers.size();

    index_buffers.emplace_back();
    auto result = index_buffers[id].init();
    if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
        if (result.has_error()) {
            return Utily::Error {
                std::format("VertexBuffer {} failed. {}", id, result.error().what())
            };
        }
    }
    return id;
}

auto AppRenderer::add_texture(Media::Image& image) noexcept -> Utily::Result<TextureId, Utily::Error> {
    auto id = textures.size();
    textures.emplace_back();

    auto& texture = textures[id];

    if (auto result = texture.init(); result.has_error()) {
        return result.error();
    }

    if (auto result = texture.upload_image(image, Renderer::Texture::Filter::smooth, false); result.has_error()) {
        return result.error();
    }
    return TextureId { static_cast<std::ptrdiff_t>(id) };
}

void AppRenderer::stop() noexcept {
    Profiler::Timer("AppRenderer::stop()");
    // explicitly destroy, just in case the gl context closes first.
    shaders.resize(0);
    vertex_buffers.resize(0);
}