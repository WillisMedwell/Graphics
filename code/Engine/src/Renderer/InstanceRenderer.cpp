#include "Renderer/InstanceRenderer.hpp"

namespace Renderer {

    // constexpr static std::string_view FBR_SHADER_VERT_SRC =
    //     "precision highp float;\n"
    //     "layout(location = 0) in vec2 l_pos;\n"
    //     "layout(location = 1) in vec2 l_uv;\n"
    //     "out vec2 uv;\n"
    //     "void main() {\n"
    //     "    gl_Position = vec4(l_pos, 0, 1.0);\n"
    //     "    uv = l_uv;\n"
    //     "}";
    // constexpr static std::string_view FBR_SHADER_FRAG_SRC =
    //     "precision highp float;\n"
    //     "uniform sampler2D u_texture;\n"
    //     "uniform vec4 u_colour;\n"
    //     "in vec2 uv;\n"
    //     "out vec4 FragColor;\n"
    //     "void main() {\n"
    //     "    vec2 uv_flipped = vec2(uv.x, 1.0f - uv.y);\n"
    //     "    float r = texture(u_texture, uv_flipped).r;"
    //     "    if(r > 0.1f) {\n"
    //     "        FragColor = vec4(u_colour.rgb, r * u_colour.a);\n"
    //     "    } else {\n"
    //     "        FragColor = vec4(0,0,0,0);"
    //     "    }\n"
    //     "}";

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
        "    mat4 mvp = u_view * u_proj * m;\n"
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

        void
        InstanceRenderer::init(ResourceManager & resource_manager, const Model::Static& model, Media::Image& image) {
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
    }
    void InstanceRenderer::stop(ResourceManager& resource_manager) {
    }
    void InstanceRenderer::push_instance(const glm::mat4& instance_transformation) {
        _current_instances.emplace_back(instance_transformation);
    }
    void InstanceRenderer::draw_instances(ResourceManager& resource_manager, glm::vec2 screen_dimensions) {
    }

}