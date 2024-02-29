#pragma once
#include "Renderer/FontRenderer.hpp"

#include "Cameras/Orthographic.hpp"
#include "Core/Core.hpp"
#include "Media/Media.hpp"

#include <span>
#include <random>

namespace Renderer {
    constexpr static std::string_view SHADER_VERT_SRC =
        "precision highp float;\n"

        "layout(location = 0) in vec2 l_pos;\n"
        "layout(location = 1) in vec2 l_uv;\n"

        "uniform mat4 u_mvp;\n"

        "out vec2 uv;\n"

        "void main() {\n"
        "    gl_Position = u_mvp * vec4(l_pos, 0, 1.0);\n"
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

    void FontRenderer::ensure_buffers_have_capacity_for(const size_t N, Core::IndexBuffer& ib) {
        Profiler::Timer timer("FontRenderer::ensure_buffers_have_capacity_for()", {});

        if (_vertices.size() >= (N * 4) || _indices.size() >= (N * 6)) {
            return;
        }

        _vertices.resize(N * 4);
        _indices.resize(N * 6);

        const float glyph_ratio = _glyph_dimensions.x / _glyph_dimensions.y;
        constexpr float min_y = 0;
        constexpr float max_y = 1;

        for (int i = 0, v = 0; i < _vertices.size(); ++v, i += 4) {
            auto vertices = std::span<FontVertex, 4> { _vertices.data() + i, 4 };

            const float min_x = glyph_ratio * v;
            const float max_x = min_x + glyph_ratio;

            vertices[0].position = { min_x, min_y };
            vertices[1].position = { max_x, min_y };
            vertices[2].position = { max_x, max_y };
            vertices[3].position = { min_x, max_y };
        }

        for (int i = 0, v = 0; i < _indices.size(); v += 4, i += 6) {
            auto indices = _indices.data() + i;
            indices[0] = v + 0;
            indices[1] = v + 1;
            indices[2] = v + 2;
            indices[3] = v + 2;
            indices[4] = v + 3;
            indices[5] = v + 0;
        }
        ib.load_indices(_indices);
    }

    auto FontRenderer::uv_coord_of_char(char a) const noexcept -> Media::FontAtlas::UvCoord {
        constexpr auto drawble_chars = Media::FontAtlasConstants::DRAWABLE_CHARS;

        assert(drawble_chars.front() <= a && a <= drawble_chars.back() && "Must be a printable character");

        const auto i = static_cast<int>(a - drawble_chars.front());

        const float r = static_cast<float>(i / _atlas_dimensions.x);
        const float c = static_cast<float>(i % static_cast<int>(_atlas_dimensions.x));

        return {
            .min_x = c / static_cast<float>(_atlas_dimensions.x),
            .max_x = (c + 1) / static_cast<float>(_atlas_dimensions.x),
            .min_y = 1 - (r + 1) / static_cast<float>(_atlas_dimensions.y),
            .max_y = 1 - r / static_cast<float>(_atlas_dimensions.y),
        };
    }

    void FontRenderer::load_text_into_vb(const std::string_view& text, Core::VertexBuffer& vb) {
        Profiler::Timer timer("FontRenderer::load_text_into_vb()", {});
        const auto text_hash = std::hash<std::string_view> {}(text);
        const auto text_size = text.size();

        if (text_hash == _loaded_text_hash) {
            return;
        }

        Media::FontAtlas fa {};

        fa.glyph_width = _glyph_dimensions.x;
        fa.glyph_height = _glyph_dimensions.y;
        fa.columns = _atlas_dimensions.x;
        fa.rows = _atlas_dimensions.y;

        for (int i = 0, v = 0; i < _vertices.size() && v < text.size(); ++v, i += 4) {
            auto vertices = std::span<FontVertex, 4> { _vertices.data() + i, 4 };
            const auto uv = fa.uv_coord_of_char(text[v]);

            vertices[0].uv_coord = { uv.min_x, uv.min_y };
            vertices[1].uv_coord = { uv.max_x, uv.min_y };
            vertices[2].uv_coord = { uv.max_x, uv.max_y };
            vertices[3].uv_coord = { uv.min_x, uv.max_y };
        }
        vb.load_vertices(std::span { _vertices.begin(), _vertices.begin() + (text.size() * 4) });
    }

    bool FontRenderer::is_init() const noexcept {
        return _glyph_dimensions.x > 0 && _glyph_dimensions.y > 0
            && _atlas_dimensions.x > 0 && _atlas_dimensions.y > 0;
    }

    struct Panic {
        void operator()(Utily::Error& error) {
            throw std::runtime_error(std::string(error.what()));
        }
    };

    void FontRenderer::init(ResourceManager& resource_manager, Media::FontAtlas& font_atlas) {

        auto [s_handle, shader] = resource_manager.create_and_init_resource<Core::Shader>(SHADER_VERT_SRC, SHADER_FRAG_SRC);
        auto [t_handle, texture] = resource_manager.create_and_init_resource<Core::Texture>();
        auto [vb_handle, vertex_buffer] = resource_manager.create_and_init_resource<Core::VertexBuffer>();
        auto [ib_handle, index_buffer] = resource_manager.create_and_init_resource<Core::IndexBuffer>();
        auto [va_handle, vertex_array] = resource_manager.create_and_init_resource<Core::VertexArray>(FontVertex::VBL {}, vertex_buffer, index_buffer);

        _s = s_handle;
        _t = t_handle;
        _vb = vb_handle;
        _ib = ib_handle;
        _va = va_handle;

        texture.upload_image(font_atlas.image).on_error(Panic {});

        _glyph_dimensions = { font_atlas.glyph_width, font_atlas.glyph_height };
        _atlas_dimensions = { font_atlas.columns, font_atlas.rows };
    }

    

    void FontRenderer::stop(ResourceManager& resource_manager) {
        resource_manager.free_resources(_s, _t, _va, _ib, _vb);
    }

    void FontRenderer::draw(ResourceManager& resource_manager, glm::vec2 screen_dimensions, std::string_view text, float char_size_px, glm::vec2 bottom_left, glm::vec4 colour) {
        Profiler::Timer timer("FontRenderer::draw()", {});
        assert(is_init());

        auto [s, t, va, vb, ib] = resource_manager.get_resources(_s, _t, _va, _vb, _ib);

        ensure_buffers_have_capacity_for(text.size(), ib);
        load_text_into_vb(text, vb);

        const int32_t texture_slot = t.bind().on_error(Panic {}).value();
        s.bind();
        va.bind();
        ib.bind();
        vb.bind();

        const auto proj_mat = Cameras::Orthographic::projection_matrix(screen_dimensions.x, screen_dimensions.y);

        auto mat = glm::mat4(1.0f);
        mat = glm::translate(mat, glm::vec3(bottom_left.x / screen_dimensions.x * 2 - 1, bottom_left.y / screen_dimensions.y * 2 - 1, 0));
        mat = glm::scale(mat, glm::vec3(char_size_px / screen_dimensions.x * 2, char_size_px / screen_dimensions.y * 2, 0.0f));

        s.set_uniform("u_texture", texture_slot).on_error(Panic {});
        s.set_uniform("u_mvp", mat).on_error(Panic {});
        s.set_uniform("u_colour", colour).on_error(Panic {});

        assert(text.size() * 6 <= ib.get_count());

        {
            Profiler::Timer draw_timer("glDrawElements()", {});
            glDisable(GL_DEPTH_TEST);
            glDrawElements(GL_TRIANGLES, text.size() * 6, GL_UNSIGNED_INT, (void*)0);
            glEnable(GL_DEPTH_TEST);
        }
    }
}