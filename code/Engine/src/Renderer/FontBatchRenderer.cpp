#include "Renderer/FontBatchRenderer.hpp"

namespace Renderer {

    constexpr static std::string_view FBR_SHADER_VERT_SRC =
        "precision highp float;\n"
        "layout(location = 0) in vec2 l_pos;\n"
        "layout(location = 1) in vec2 l_uv;\n"

        "out vec2 uv;\n"

        "void main() {\n"
        "    gl_Position = vec4(l_pos, 0, 1.0);\n"
        "    uv = l_uv;\n"
        "}";
    constexpr static std::string_view FBR_SHADER_FRAG_SRC =
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

    void FontBatchRenderer::load_text_into_vb(const std::string_view& text, glm::vec2 bottom_left, float height_px) {
        int v = static_cast<int>(_m.current_batch_vertices.size());
        _m.current_batch_vertices.resize(_m.current_batch_vertices.size() + text.size() * 4);

        const float glyph_ratio = _m.font_atlas.glyph_dimensions().x / _m.font_atlas.glyph_dimensions().y;

        for (int t = 0; t < text.size(); ++t, v += 4) {
            Vertex* vertices = _m.current_batch_vertices.data() + v;

            const auto uv = _m.font_atlas.uv_for(text[t]);

            // translate it by screen coords
            const float translated_min_x = glyph_ratio * t * height_px + bottom_left.x;
            const float translated_max_x = (glyph_ratio * t + glyph_ratio) * height_px + bottom_left.x;
            const float translated_min_y = bottom_left.y;
            const float translated_max_y = height_px + bottom_left.y;

            // scale it to screen coords [-1, 1]
            const float actual_min_x = translated_min_x / _m.current_batch_config->screen_dimensions.x * 2 - 1;
            const float actual_max_x = translated_max_x / _m.current_batch_config->screen_dimensions.x * 2 - 1;
            const float actual_min_y = translated_min_y / _m.current_batch_config->screen_dimensions.y * 2 - 1;
            const float actual_max_y = translated_max_y / _m.current_batch_config->screen_dimensions.y * 2 - 1;

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

    auto FontBatchRenderer::create(ResourceManager& resource_manager, std::filesystem::path ttf_path) noexcept -> Utily::Result<FontBatchRenderer, Utily::Error> {

        auto font_atlas_result = Media::FontAtlas::create(ttf_path, 300);
        if (font_atlas_result.has_error()) {
            return font_atlas_result.error();
        }
        auto& font_atlas = font_atlas_result.value();

        auto [s_handle, shader] = resource_manager.create_and_init_resource<Core::Shader>(FBR_SHADER_VERT_SRC, FBR_SHADER_FRAG_SRC);
        auto [t_handle, texture] = resource_manager.create_and_init_resource<Core::Texture>();
        auto [vb_handle, vertex_buffer] = resource_manager.create_and_init_resource<Core::VertexBuffer>();
        auto [ib_handle, index_buffer] = resource_manager.create_and_init_resource<Core::IndexBuffer>();
        auto [va_handle, vertex_array] = resource_manager.create_and_init_resource<Core::VertexArray>(Vertex::VBL {}, vertex_buffer, index_buffer);

        auto texture_upload_result = texture.upload_image(font_atlas.atlas_image());
        if (texture_upload_result.has_error()) {
            return texture_upload_result.error();
        }

        return FontBatchRenderer(M {
            .current_batch_vertices = {},
            .current_batch_config = std::nullopt,
            .font_atlas = std::move(font_atlas),
            .s = s_handle,
            .t = t_handle,
            .vb = vb_handle,
            .ib = ib_handle,
            .va = va_handle,
        });
    }

    void FontBatchRenderer::begin_batch(BatchConfig&& batch_config) {
        assert(!_m.current_batch_config);
        _m.current_batch_config.emplace(std::move(batch_config));
    }
    void FontBatchRenderer::push_to_batch(std::string_view text, glm::vec2 bottom_left, float height_px) {
        Profiler::Timer timer("FontBatchRenderer::push_to_batch()", {});
        assert(_m.current_batch_config);
        load_text_into_vb(text, bottom_left, height_px);
    }
    void FontBatchRenderer::end_batch() {
        // 1. Validate a batch config has been passed in.
        // 2. Get resources, bind, set uniforms, load vertices.
        // 3. Ensure the index buffer has enough loaded for the vertex buffer.
        // 4. Disable depth testing, draw, and then re-enable depth testing.
        // 5. Clear batch's config and vertices.

        // 1.
        Profiler::Timer timer("FontBatchRenderer::end_batch()", {});
        assert(_m.current_batch_config);
        if (_m.current_batch_vertices.size() == 0) {
            assert(false && "redundant batching");
            return;
        }

        // 2.
        auto [s, t, va, vb, ib] = _m.current_batch_config->resource_manager.get_resources(_m.s, _m.t, _m.va, _m.vb, _m.ib);
        const int32_t texture_slot = t.bind().value();
        s.bind();
        va.bind();
        ib.bind();
        vb.bind();

        s.set_uniform("u_texture", texture_slot).on_error(Panic {});
        s.set_uniform("u_colour", _m.current_batch_config->font_colour).on_error(Panic {});
        vb.load_vertices(_m.current_batch_vertices);

        // 3.
        if (ib.get_count() < _m.current_batch_vertices.size() / 4 * 6) {
            std::vector<Model::Index> indices;
            indices.resize(_m.current_batch_vertices.size() / (size_t)4 * (size_t)6, 0);

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

        // 4.
        {
            Profiler::Timer draw_timer("glDrawElements()", {});
            glDisable(GL_DEPTH_TEST);
            glDrawElements(GL_TRIANGLES, _m.current_batch_vertices.size() / 4 * 6, GL_UNSIGNED_INT, (void*)0);
            glEnable(GL_DEPTH_TEST);
        }

        // 5.
        _m.current_batch_config = std::nullopt;
        _m.current_batch_vertices.resize(0);
    }
}