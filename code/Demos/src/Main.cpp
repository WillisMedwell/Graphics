#include "App.hpp"
#include "Components/Components.hpp"
#include "Config.hpp"
#include "Model/Model.hpp"

#include <chrono>
#include <iostream>
#include <span>
#include <string_view>

#include <Utily/Utily.hpp>

using namespace std::literals;

void* operator new(std::size_t size) {
    std::cout << "Custom new called, size = " << size << std::endl;
    void* memory = std::malloc(size);
    return memory;
}

void* operator new[](std::size_t size) {
    std::cout << "Custom new[] called, size = " << size << std::endl;
    void* memory = std::malloc(size);
    return memory;
}

struct SpinningTeapotData {
    entt::entity teapot;
    Cameras::StationaryPerspective camera { glm::vec3(0, 1, -1), glm::normalize(glm::vec3(0, -0.25f, 1)) };

    AppRenderer::ShaderId s_id;
    AppRenderer::IndexBufferId ib_id;
    AppRenderer::VertexBufferId vb_id;
    AppRenderer::VertexArrayId va_id;

    constexpr static std::string_view VERT =
        "precision highp float; "
        "uniform mat4 u_mvp;"
        "layout(location = 0) in vec3 aPos;"
        "layout(location = 1) in vec3 aNor;"
        "layout(location = 2) in vec2 aUv;"
        "out vec3 Normal;"
        "void main() {"
        "    gl_Position = u_mvp * vec4(aPos, 1.0);"
        "    Normal = aNor;"
        "}"sv;

    constexpr static std::string_view FRAG =
        "precision highp float; "
        "in vec3 Normal;"
        "out vec4 FragColor;"
        "void main() {"
        "    vec3 color = Normal * 0.5 + 0.5; "
        "    FragColor = vec4(color, 1.0); "
        "}"sv;
};
struct SpinningTeapotLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, SpinningTeapotData& data) {
        auto teapot_source = Utily::FileReader::load_entire_file("assets/teapot.obj")
                                 .on_error(Utily::ErrorHandler::print_then_quit)
                                 .value();
        auto teapot_model = std::move(Model::decode_as_static_model(teapot_source, { '.', 'o', 'b', 'j' })
                                          .on_error(Utily::ErrorHandler::print_then_quit)
                                          .value());

        data.teapot = ecs.create();
        ecs.emplace<Model::Static>(data.teapot, std::move(teapot_model));
        ecs.emplace<Components::Transform>(data.teapot, glm::vec3 { 0, -1, 1 }, glm::vec3(0.5f));
        ecs.emplace<Components::Spinning>(data.teapot, glm::vec3 { 0, 1, 0 }, 0.0, 0.5);

        renderer.add_shader(data.VERT, data.FRAG)
            .on_error(Utily::ErrorHandler::print_then_quit)
            .on_value([&](auto& id) {
                data.s_id = id;
            });
        renderer.add_vertex_buffer()
            .on_error(Utily::ErrorHandler::print_then_quit)
            .on_value([&](auto& id) {
                data.vb_id = id;
            });
        renderer.add_index_buffer()
            .on_error(Utily::ErrorHandler::print_then_quit)
            .on_value([&](auto& id) {
                data.ib_id = id;
            });
        renderer.add_vertex_array(Model::Vertex::VBL {}, data.vb_id)
            .on_error(Utily::ErrorHandler::print_then_quit)
            .on_value([&](auto& id) {
                data.va_id = id;
            });
    }
    void update(double dt, AppInput& input, AppState& state, entt::registry& ecs, SpinningTeapotData& data) {
        ecs.get<Components::Transform>(data.teapot).rotation = ecs.get<Components::Spinning>(data.teapot)
                                                                   .update(dt)
                                                                   .calc_quat();
    }

    void draw(AppRenderer& renderer, entt::registry& ecs, SpinningTeapotData& data) {
        auto [model, transform, spinning] = ecs.get<Model::Static, Components::Transform, Components::Spinning>(data.teapot);

        transform.rotation = spinning.calc_quat();
        auto pm = data.camera.projection_matrix(renderer.window_width, renderer.window_height);
        auto vm = data.camera.view_matrix();
        auto mm = transform.calc_transform_mat();

        auto mvp = pm * vm * mm;

        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear();
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Renderer::IndexBuffer& ib = renderer.index_buffers[data.ib_id.id];
        Renderer::VertexBuffer& vb = renderer.vertex_buffers[data.vb_id.id];
        Renderer::VertexArray& va = renderer.vertex_arrays[data.va_id.id];
        Renderer::Shader& s = renderer.shaders[data.s_id.id];

        s.bind();
        s.set_uniform("u_mvp", mvp);
        va.bind();
        ib.bind();
        vb.bind();
        ib.load_indices(model.indices);
        vb.load_vertices(model.vertices);
        glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, (void*)0);
        vb.bind();
    }

    void stop() {
    }
};

int main() {
    auto_run_app<SpinningTeapotData, SpinningTeapotLogic>("Demo", 1000, 500);
    return 0;
}