#include "Renderer/InstanceRenderer.hpp"

namespace Renderer {
    constexpr static std::string_view INSTANCE_SHADER_VERT_SRC =
        "precision highp float;\n"
        "layout(location = 0) in vec3 l_pos;\n"
        "layout(location = 1) in vec3 l_norm;\n"
        "layout(location = 2) in vec2 l_uv;\n"
        "layout(location = 3) in vec4 l_inst_col_0;\n"
        "layout(location = 4) in vec4 l_inst_col_1;\n"
        "layout(location = 5) in vec4 l_inst_col_2;\n"
        "layout(location = 6) in vec4 l_inst_col_3;\n"

        "out vec2 uv;\n"

        "uniform mat4 u_view;\n"
        "uniform mat4 u_proj;\n"

        "void main() {\n"
        "    mat4 m = mat4(l_inst_col_0, l_inst_col_1, l_inst_col_2, l_inst_col_3);"
        "    mat4 mvp = u_proj * u_view * m;\n"
        "    gl_Position =  mvp * vec4(l_pos, 1);\n"
        "    uv = l_uv;\n"
        "}";

    constexpr static std::string_view INSTANCE_SHADER_FRAG_SRC =
        "precision highp float;\n"
        "in vec2 uv;\n"
        "uniform sampler2D u_texture;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "        FragColor = texture(u_texture, uv);\n"
        "}";

    void InstanceRenderer::init(ResourceManager& resource_manager, const Model::Static& model, Media::Image& image) {
        auto [ib_handle, ib] = resource_manager.create_and_init_resource<Core::IndexBuffer>();
        auto [vb_mesh_handle, vb_mesh] = resource_manager.create_and_init_resource<Core::VertexBuffer>();
        auto [vb_tran_handle, vb_tran] = resource_manager.create_and_init_resource<Core::VertexBuffer>();
        auto [va_handle, va] = resource_manager.create_and_init_resource<Core::VertexArray>(Model::Vertex::VBL {}, vb_mesh, vb_tran, ib);
        auto [s_handle, s] = resource_manager.create_and_init_resource<Core::Shader>(INSTANCE_SHADER_VERT_SRC, INSTANCE_SHADER_FRAG_SRC);
        auto [t_handle, t] = resource_manager.create_and_init_resource<Core::Texture>();
        t.upload_image(image).on_error(Renderer::Panic {});

        _s = s_handle;
        _t = t_handle;
        _vb_mesh = vb_mesh_handle;
        _vb_transforms = vb_tran_handle;
        _ib = ib_handle;
        _va = va_handle;

        vb_mesh.bind();
        vb_mesh.load_vertices(model.vertices);

        ib.bind();
        ib.load_indices(model.indices);

        va.unbind();
    }
    void InstanceRenderer::stop(ResourceManager& resource_manager) {
    }
    void InstanceRenderer::push_instance(const glm::mat4& instance_transformation) {
        _current_instances.emplace_back(instance_transformation);
    }
    void InstanceRenderer::draw_instances(ResourceManager& resource_manager, const glm::mat4& projection, const glm::mat4& view) {
        auto [s, t, ib, vbm, vbt, va] = resource_manager.get_resources(_s, _t, _ib, _vb_mesh, _vb_transforms, _va);

        auto transfrom_verts = std::span {
            reinterpret_cast<const float*>(_current_instances.data()),
            _current_instances.size() * 16
        };
        va.bind();
        ib.bind();
        vbt.bind();
        vbt.load_vertices(transfrom_verts);
        vbm.bind();
        s.bind();
        s.set_uniform("u_view", view).on_error(Panic{});
        s.set_uniform("u_proj", projection).on_error(Panic{});

        int32_t t_id = static_cast<int32_t>(t.bind().on_error(Panic {}).value());
        s.set_uniform("u_texture", t_id);
        glDrawElementsInstanced(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, 0, _current_instances.size());
        va.unbind();
        _current_instances.clear();
    }

}