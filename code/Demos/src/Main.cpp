#include "App.hpp"
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

struct Data {
    std::chrono::steady_clock::time_point start_time;
    constexpr static auto TRIANGLE_VERTICES = std::to_array({ 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f });
    constexpr static auto TRIANGLE_INDICES = std::to_array<uint32_t>({ 0, 1, 3, 1, 2, 3 });

    AppRenderer::ShaderId s_id;
    AppRenderer::IndexBufferId ib_id;
    AppRenderer::VertexBufferId vb_id;
    AppRenderer::VertexArrayId va_id;

    Cameras::StationaryPerspective camera { glm::vec3(0, 0, -1), glm::vec3(0, 0, 1) };

    constexpr static glm::vec4 BACKGROUND_COLOUR = { 0, 0, 0, 1 };

    constexpr static glm::vec3 ROTATION_AXIS = { 0, 0, 1.0f };
    double angle = 0.0f;
    constexpr static double ROTATIONS_PER_SECOND = 1;
};

struct Logic {
    void init(AppRenderer& renderer, Data& data) {
        data.start_time = std::chrono::high_resolution_clock::now();
        constexpr auto vert =
            "precision highp float; "
            "uniform mat4 u_mvp;"
            "layout(location = 0) in vec3 aPos;"
            "void main() {"
            "    gl_Position = u_mvp * vec4(aPos, 1.0);"
            "}"sv;

        constexpr auto frag =
            "precision highp float; "
            "out vec4 FragColor;  "
            " void main()"
            " {"
            "     FragColor = vec4(1.0, 0.5, 0.2, 1.0); "
            " }"sv;

        renderer.add_shader(vert, frag)
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

        renderer.add_vertex_array(Renderer::VertexBufferLayout<glm::vec3> {}, data.vb_id)
            .on_error(Utily::ErrorHandler::print_then_quit)
            .on_value([&](auto& id) {
                data.va_id = id;
            });
    }
    void update(double dt, AppInput& input, AppState& state, Data& data) {

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);
        data.angle = data.angle + data.ROTATIONS_PER_SECOND * 360.0 * dt;
    }

    void draw(AppRenderer& renderer, Data& data) {
        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear(data.BACKGROUND_COLOUR);
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Renderer::IndexBuffer& ib = renderer.index_buffers[data.ib_id.id];
        Renderer::VertexBuffer& vb = renderer.vertex_buffers[data.vb_id.id];
        Renderer::VertexArray& va = renderer.vertex_arrays[data.va_id.id];
        Renderer::Shader& s = renderer.shaders[data.s_id.id];

        auto pm = data.camera.projection_matrix(renderer.window_width, renderer.window_height);
        auto vm = data.camera.view_matrix();
        auto mm = glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(data.angle)), data.ROTATION_AXIS);

        auto mvp = pm * vm * mm;

        s.bind();
        s.set_uniform("u_mvp", mvp);
        va.bind();
        ib.bind();
        vb.bind();
        ib.load_indices(data.TRIANGLE_INDICES);
        vb.load_vertices(data.TRIANGLE_VERTICES);
        glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, (void*)0);
    }

    void stop() {
    }
};

int main() {
    auto_run_app<Data, Logic>("Demo", 1000, 500);
    return 0;
}