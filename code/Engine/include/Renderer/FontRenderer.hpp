#pragma once

#include "Core/Core.hpp"
#include "Media/Media.hpp"
#include "Renderer/ResourceManager.hpp"

namespace Renderer {
    class FontRenderer
    {
    private:
        Renderer::ResourceHandle<Core::Shader> _s;
        Renderer::ResourceHandle<Core::Texture> _t;
        Renderer::ResourceHandle<Core::VertexBuffer> _vb;
        Renderer::ResourceHandle<Core::IndexBuffer> _ib;
        Renderer::ResourceHandle<Core::VertexArray> _va;

        struct FontVertex {
            glm::vec2 position;
            glm::vec2 uv_coord;

            using VBL = Core::VertexBufferLayout<glm::vec2, glm::vec2>;
        };

        size_t _loaded_text_hash = 0;
        
        std::vector<FontVertex> _vertices;
        std::vector<Model::Index> _indices;

        glm::vec2 _glyph_dimensions = { 0, 0 };
        glm::vec2 _atlas_dimensions = { 0, 0 };

        void ensure_buffers_have_capacity_for(const size_t N, Core::IndexBuffer& ib);
        void load_text_into_vb(const std::string_view& text, Core::VertexBuffer& vb);
        bool is_init() const noexcept;
        auto uv_coord_of_char(char a) const noexcept -> Media::FontAtlas::UvCoord;

    public:
        void init(ResourceManager& resource_manager, Media::FontAtlas& font_atlas);
        void stop(ResourceManager& resource_manager);
        void draw(ResourceManager& resource_manager, glm::vec2 screen_width, std::string_view text, float char_size_px, glm::vec2 bottom_left, glm::vec4 colour = { 1, 1, 1, 1 });
    };
}