
#include "Core/Core.hpp"
#include "Media/Media.hpp"
#include "Renderer/ResourceManager.hpp"

namespace Renderer {
    class FontBatchRenderer
    {
    public:
        struct BatchConfig {
            ResourceManager& resource_manager;
            glm::vec2 screen_dimensions;
            glm::vec4 font_colour;
        };
        void init(ResourceManager& resource_manager, Media::FontAtlas& font_atlas);
        
        void begin_batch(BatchConfig&& batch_config);
        void push_to_batch(std::string_view text, glm::vec2 bottom_left, float height_px);
        void end_batch();

    private:
        void load_text_into_vb(const std::string_view& text, glm::vec2 bottom_left, float height_px);

        struct Vertex {
            glm::vec2 position;
            glm::vec2 uv_coord;
            using VBL = Core::VertexBufferLayout<glm::vec2, glm::vec2>;
        };
        
        std::optional<BatchConfig> _current_batch_config = std::nullopt;
        std::vector<Vertex> _current_batch_vertices = {};

        glm::vec2 _glyph_dimensions = { 0, 0 };
        glm::vec2 _atlas_dimensions = { 0, 0 };

        Renderer::ResourceHandle<Core::Shader> _s;
        Renderer::ResourceHandle<Core::Texture> _t;
        Renderer::ResourceHandle<Core::VertexBuffer> _vb;
        Renderer::ResourceHandle<Core::IndexBuffer> _ib;
        Renderer::ResourceHandle<Core::VertexArray> _va;
    };
}