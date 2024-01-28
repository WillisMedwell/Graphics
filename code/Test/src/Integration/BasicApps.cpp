#include <gtest/gtest.h>

#include <App.hpp>
#include <iostream>

using namespace std::literals;

#include "AppAnalytics.hpp"

struct BackgroundColourData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 0, 0, 0, 1.0f };
};
struct BackgroundColourLogic {
    void init(AppRenderer& renderer, BackgroundColourData& data) {
        data.start_time = std::chrono::high_resolution_clock::now();

        constexpr auto vert =
            "precision mediump float; "
            "layout(location = 0) in vec3 aPos;"
            "void main() {"
            "    gl_Position = vec4(aPos, 1.0);"
            "}"sv;

        constexpr auto frag =
            "precision mediump float; "
            "out vec4 FragColor;  "
            " void main()"
            " {"
            "     FragColor = vec4(1.0, 0.5, 0.2, 1.0); "
            " }"sv;
        using VBL = Renderer::VertexBufferLayout<float, uint32_t, glm::vec3>;

        EXPECT_FALSE(renderer.add_shader(vert, frag).has_error());
        EXPECT_FALSE(renderer.add_vertex_buffer().has_error());
        EXPECT_FALSE(renderer.add_index_buffer().has_error());
        EXPECT_FALSE(renderer.add_vertex_array(VBL {}, renderer.add_vertex_buffer().value()).has_error());
    }
    void update(float dt, AppInput& input, AppState& state, BackgroundColourData& data) {

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);

        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }

        float increment = dt * 5.0f;

        if (data.background_colour.r < 1.0f) {
            data.background_colour.r += increment;
        } else if (data.background_colour.g < 1.0f) {
            data.background_colour.g += increment;
        } else if (data.background_colour.b < 1.0f) {
            data.background_colour.b += increment;
        } else {
            data.background_colour = { 0, 0, 0, 1.0f };
        }
    }

    void draw(AppRenderer& renderer, BackgroundColourData& data) {
        renderer.screen_frame_buffer.clear(data.background_colour);
    }

    void stop() {
    }
};

struct SpinningSquareData {
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
    constexpr static double ROTATIONS_PER_SECOND = 1;
    double angle = 0.0f;
};

struct SpinningSquareLogic {
    void init(AppRenderer& renderer, SpinningSquareData& data) {
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
    void update(double dt, AppInput& input, AppState& state, SpinningSquareData& data) {

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);
        data.angle = data.angle + data.ROTATIONS_PER_SECOND * 360.0 * dt;

        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }
    }

    void draw(AppRenderer& renderer, SpinningSquareData& data) {
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

#if defined(CONFIG_TARGET_NATIVE)
TEST(BasicApps, background_colour) {
    auto_run_app<BackgroundColourData, BackgroundColourLogic>("Test App: Background Colour ");
}
TEST(BasicApps, spinning_square) {
    auto_run_app<SpinningSquareData, SpinningSquareLogic>("Test App: Spinning Square");
}

#elif defined(CONFIG_TARGET_WEB)

void run_spinning_square() {
    static App<SpinningSquareData, SpinningSquareLogic> ss_app;
    ss_app.init("spinner", 400, 400);
    emscripten_set_main_loop(
        []() {
            if (!ss_app.is_running()) {
                emscripten_cancel_main_loop();
            }
            ss_app.poll_events();
            ss_app.update();
            ss_app.render();
        },
        0,
        1);
}

void run_background_colour() {
    static App<BackgroundColourData, BackgroundColourLogic> b_app;
    b_app.init("colour", 400, 400);
    emscripten_set_main_loop(
        []() {
            if (!b_app.is_running()) {
                emscripten_cancel_main_loop();
                run_spinning_square();
            }
            b_app.poll_events();
            b_app.update();
            b_app.render();
        },
        0,
        1);
}

TEST(BasicApps, apps) {
    run_background_colour();
}

#endif