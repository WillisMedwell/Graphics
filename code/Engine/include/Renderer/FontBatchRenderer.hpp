
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

        static auto create(ResourceManager& resource_manager, std::filesystem::path ttf_path) noexcept -> Utily::Result<FontBatchRenderer, Utily::Error>;

        void begin_batch(BatchConfig&& batch_config);
        void push_to_batch(std::string_view text, glm::vec2 bottom_left, float height_px);
        void end_batch();

        FontBatchRenderer(FontBatchRenderer&& other)
            : _m(std::move(other._m)) { }

    private:
        void load_text_into_vb(const std::string_view& text, glm::vec2 bottom_left, float height_px);

        struct Vertex {
            glm::vec2 position;
            glm::vec2 uv_coord;
            using VBL = Core::VertexBufferLayout<glm::vec2, glm::vec2>;
        };

        struct M {
            std::vector<Vertex> current_batch_vertices;
            std::optional<BatchConfig> current_batch_config;

            Media::FontAtlas font_atlas;

            Renderer::ResourceHandle<Core::Shader> s;
            Renderer::ResourceHandle<Core::Texture> t;
            Renderer::ResourceHandle<Core::VertexBuffer> vb;
            Renderer::ResourceHandle<Core::IndexBuffer> ib;
            Renderer::ResourceHandle<Core::VertexArray> va;
        } _m;

        explicit FontBatchRenderer(M&& m)
            : _m(std::move(m)) { }
    };
}