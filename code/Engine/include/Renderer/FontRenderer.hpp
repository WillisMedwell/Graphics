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
        size_t _loaded_text_size = 0;
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

    class FontBatchRenderer
    {
    private:
        constexpr static std::string_view SHADER_VERT_SRC =
            "precision highp float;\n"

            "layout(location = 0) in vec2 l_pos;\n"
            "layout(location = 1) in vec2 l_uv;\n"

            "out vec2 uv;\n"

            "void main() {\n"
            "    gl_Position = vec4(l_pos, 0, 1.0);\n"
            "    uv = l_uv;\n"
            "}";
        constexpr static std::string_view SHADER_FRAG_SRC =
            "precision highp float;\n"

            "uniform sampler2D u_texture;\n"
            "uniform vec4 u_colour;\n"

            "in vec2 uv;\n"
            "out vec4 FragColor;\n"

            "void main() {\n"
            "    vec2 uv_flipped = vec2(uv.x, 1.0f - uv.y);\n"
            "    float r = texture(u_texture, uv_flipped).r;"
            "    if(r > 0.1f) {\n"
            "        FragColor = vec4(u_colour.rgb, r * u_colour.a);\n"
            "    } else {\n"
            "        FragColor = vec4(0,0,0,0);"
            "    }\n"
            "}";

        struct Vertex {
            glm::vec2 position;
            glm::vec2 uv_coord;
            using VBL = Core::VertexBufferLayout<glm::vec2, glm::vec2>;
        };

        void load_text_into_vb(const std::string_view& text, glm::vec2 bottom_left, float height_px) {
            int v = static_cast<int>(_current_batch_vertices.size());
            _current_batch_vertices.resize(_current_batch_vertices.size() + text.size() * 4);

            Media::FontAtlas font_atlas;
            font_atlas.glyph_width = _glyph_dimensions.x;
            font_atlas.glyph_height = _glyph_dimensions.y;
            font_atlas.columns = _atlas_dimensions.x;
            font_atlas.rows = _atlas_dimensions.y;

            const float glyph_ratio = _glyph_dimensions.x / _glyph_dimensions.y;

            for (int t = 0; t < text.size(); ++t, v += 4) {
                Vertex* vertices = _current_batch_vertices.data() + v;

                const auto uv = font_atlas.uv_coord_of_char(text[t]);

                // translate it by screen coords
                const float translated_min_x = glyph_ratio * t * height_px + bottom_left.x;
                const float translated_max_x = (glyph_ratio * t + glyph_ratio) * height_px + bottom_left.x;
                const float translated_min_y = bottom_left.y;
                const float translated_max_y = height_px + bottom_left.y;

                // scale it to screen coords [-1, 1]
                const float actual_min_x = translated_min_x / _current_batch_config->screen_dimensions.x * 2 - 1;
                const float actual_max_x = translated_max_x / _current_batch_config->screen_dimensions.x * 2 - 1;
                const float actual_min_y = translated_min_y / _current_batch_config->screen_dimensions.y * 2 - 1;
                const float actual_max_y = translated_max_y / _current_batch_config->screen_dimensions.y * 2 - 1;

                vertices[0] = {
                    .position = { actual_min_x, actual_min_y },
                    .uv_coord = { uv.min_x, uv.min_y }
                };
                vertices[1] = {
                    .position = { actual_max_x, actual_min_y },
                    .uv_coord = { uv.max_x, uv.min_y }
                };
                vertices[2] = {
                    .position = { actual_max_x, actual_max_y },
                    .uv_coord = { uv.max_x, uv.max_y }
                };
                vertices[3] = {
                    .position = { actual_min_x, actual_max_y },
                    .uv_coord = { uv.min_x, uv.max_y }
                };
            }
        }

    public:
        struct BatchConfig {
            ResourceManager& resource_manager;
            glm::vec2 screen_dimensions;
            glm::vec4 font_colour;
        };

        void init(ResourceManager& resource_manager, Media::FontAtlas& font_atlas) {
            auto [s_handle, shader] = resource_manager.create_and_init_resource<Core::Shader>(SHADER_VERT_SRC, SHADER_FRAG_SRC);
            auto [t_handle, texture] = resource_manager.create_and_init_resource<Core::Texture>();
            auto [vb_handle, vertex_buffer] = resource_manager.create_and_init_resource<Core::VertexBuffer>();
            auto [ib_handle, index_buffer] = resource_manager.create_and_init_resource<Core::IndexBuffer>();
            auto [va_handle, vertex_array] = resource_manager.create_and_init_resource<Core::VertexArray>(Vertex::VBL {}, vertex_buffer, index_buffer);

            _s = s_handle;
            _t = t_handle;
            _vb = vb_handle;
            _ib = ib_handle;
            _va = va_handle;

            texture.upload_image(font_atlas.image); //.on_error(Panic {});

            _glyph_dimensions = { font_atlas.glyph_width, font_atlas.glyph_height };
            _atlas_dimensions = { font_atlas.columns, font_atlas.rows };
        }

        void begin_batch(BatchConfig&& batch_config) {
            assert(!_current_batch_config);
            _current_batch_config.emplace(std::move(batch_config));
        }
        void push_to_batch(std::string_view text, glm::vec2 bottom_left, float height_px) {
            Profiler::Timer timer("FontBatchRenderer::push_to_batch()", {});

            assert(_current_batch_config);
            load_text_into_vb(text, bottom_left, height_px);
        }
        void end_batch() {
            Profiler::Timer timer("FontBatchRenderer::end_batch()", {});
            assert(_current_batch_config);
            if (_current_batch_vertices.size() == 0) {
                assert(false && "redundant batching");
                return;
            }

            // bind everything
            auto [s, t, va, vb, ib] = _current_batch_config->resource_manager.get_resources(_s, _t, _va, _vb, _ib);
            const int32_t texture_slot = t.bind().value();
            s.bind();
            va.bind();
            ib.bind();
            vb.bind();

            s.set_uniform("u_texture", texture_slot);
            s.set_uniform("u_colour", _current_batch_config->font_colour);

            // load vertices
            vb.load_vertices(_current_batch_vertices);

            // ensure ib has enough capacity/indices for vb
            if (ib.get_count() < _current_batch_vertices.size() / 4 * 6) {
                std::vector<Model::Index> indices;
                indices.resize(_current_batch_vertices.size() / (size_t)4 * (size_t)6, 0);

                for (int v = 0, i = 0; i < indices.size(); i += 6, v += 4) {
                    indices[i + 0] = v + 0;
                    indices[i + 1] = v + 1;
                    indices[i + 2] = v + 2;
                    indices[i + 3] = v + 2;
                    indices[i + 4] = v + 3;
                    indices[i + 5] = v + 0;
                }
                assert(indices.size());
                ib.load_indices(indices);
            }

            // gldraw indices = vb.vertices / 4 * 6
            {
                Profiler::Timer draw_timer("glDrawElements()", {});
                glDisable(GL_DEPTH_TEST);
                glDrawElements(GL_TRIANGLES, _current_batch_vertices.size() / 4 * 6, GL_UNSIGNED_INT, (void*)0);
                glEnable(GL_DEPTH_TEST);
            }

            _current_batch_config = std::nullopt;
            _current_batch_vertices.resize(0);
        }

    private:
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