#pragma once

#include "TestPch.hpp"

#include <iostream>
#include <thread>

#include <Renderer/Renderer.hpp>

#include <Engine.hpp>

using namespace std::literals;

struct BackgroundColourData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 0, 0, 0, 1.0f };
};
struct BackgroundColourLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, BackgroundColourData& data) {
        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(float dt, const Core::InputManager& input, AppState& state, entt::registry& ecs, BackgroundColourData& data) {

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
    void draw(AppRenderer& renderer, entt::registry& ecs, BackgroundColourData& data) {
        renderer.screen_frame_buffer.clear(data.background_colour);
    }
    void stop() {
    }
};

struct SpinningSquareData {
    std::chrono::steady_clock::time_point start_time;
    constexpr static auto TRIANGLE_VERTICES = std::to_array({ 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f });
    constexpr static auto TRIANGLE_INDICES = std::to_array<uint32_t>({ 0, 1, 3, 1, 2, 3 });

    Renderer::ResourceManager resource_manager;

    AppRenderer::ShaderId s_id;
    AppRenderer::IndexBufferId ib_id;
    AppRenderer::VertexBufferId vb_id;
    AppRenderer::VertexArrayId va_id;

    Cameras::StationaryPerspective camera { glm::vec3(0, 0, -1), glm::vec3(0, 0, 1) };

    constexpr static glm::vec4 BACKGROUND_COLOUR = { 0, 0, 0, 1 };
    constexpr static glm::vec3 ROTATION_AXIS = { 0, 0, 1.0f };
    constexpr static double ROTATIONS_PER_SECOND = 1;
    double angle = 0.0f;

    constexpr static std::string_view VERT =
        "precision highp float; "
        "uniform mat4 u_mvp;"
        "layout(location = 0) in vec3 aPos;"
        "void main() {"
        "    gl_Position = u_mvp * vec4(aPos, 1.0);"
        "}"sv;

    constexpr static std::string_view FRAG =
        "precision highp float; "
        "out vec4 FragColor;  "
        " void main()"
        " {"
        "     FragColor = vec4(1.0, 0.5, 0.2, 1.0); "
        " }"sv;
};
struct SpinningSquareLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, SpinningSquareData& data) {
        data.start_time = std::chrono::high_resolution_clock::now();

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

        renderer.add_vertex_array(Core::VertexBufferLayout<glm::vec3> {}, data.vb_id, data.ib_id)
            .on_error(Utily::ErrorHandler::print_then_quit)
            .on_value([&](auto& id) {
                data.va_id = id;
            });

        auto [vb_h, vb] = data.resource_manager.create_and_init_resource<Core::VertexBuffer>();
        auto [ib_h, ib] = data.resource_manager.create_and_init_resource<Core::IndexBuffer>();
        auto [va_h, va] = data.resource_manager.create_and_init_resource<Core::VertexArray>(Model::Vertex::VBL {}, vb, ib);
    }
    void update(double dt, const Core::InputManager& input, AppState& state, entt::registry& ecs, SpinningSquareData& data) {

        data.angle = data.angle + data.ROTATIONS_PER_SECOND * 360.0 * dt;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);

        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }
    }

    void draw(AppRenderer& renderer, entt::registry& ecs, SpinningSquareData& data) {
        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear(data.BACKGROUND_COLOUR);
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Core::IndexBuffer& ib = renderer.index_buffers[data.ib_id.id];
        Core::VertexBuffer& vb = renderer.vertex_buffers[data.vb_id.id];
        Core::VertexArray& va = renderer.vertex_arrays[data.va_id.id];
        Core::Shader& s = renderer.shaders[data.s_id.id];

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

struct SpinningTeapotData {
    std::chrono::steady_clock::time_point start_time;
    entt::entity teapot;
    Cameras::StationaryPerspective camera { glm::vec3(0, 1, -1), glm::normalize(glm::vec3(0, -0.25f, 0.5f)) };

    Renderer::ResourceManager resource_manager;
    Renderer::ResourceHandle<Core::Shader> s_h;
    Renderer::ResourceHandle<Core::VertexBuffer> vb_h;
    Renderer::ResourceHandle<Core::IndexBuffer> ib_h;
    Renderer::ResourceHandle<Core::VertexArray> va_h;

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

        auto print_pause_quit = [&](auto error) {
            std::cerr << error.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            exit(-1);
        };

        auto teapot_source = Utily::FileReader::load_entire_file("assets/teapot.obj")
                                 .on_error(print_pause_quit)
                                 .value();

        auto teapot_model = std::move(Model::decode_as_static_model(teapot_source, { '.', 'o', 'b', 'j' })
                                          .on_error(print_pause_quit)
                                          .value());

        data.teapot = ecs.create();
        ecs.emplace<Model::Static>(data.teapot, std::move(teapot_model));
        ecs.emplace<Components::Transform>(data.teapot, glm::vec3 { 0, -1, 1 }, glm::vec3(0.5f));
        ecs.emplace<Components::Spinning>(data.teapot, glm::vec3 { 0, 1, 0 }, 0.0, 1.0);

        auto [s_h, s] = data.resource_manager.create_and_init_resource<Core::Shader>(data.VERT, data.FRAG);
        auto [vb_h, vb] = data.resource_manager.create_and_init_resource<Core::VertexBuffer>();
        auto [ib_h, ib] = data.resource_manager.create_and_init_resource<Core::IndexBuffer>();
        auto [va_h, va] = data.resource_manager.create_and_init_resource<Core::VertexArray>(Model::Vertex::VBL {}, vb, ib);

        data.s_h = s_h;
        data.vb_h = vb_h;
        data.va_h = va_h;
        data.ib_h = ib_h;

        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(double dt, const Core::InputManager& input, AppState& state, entt::registry& ecs, SpinningTeapotData& data) {
        ecs.get<Components::Transform>(data.teapot).rotation = ecs.get<Components::Spinning>(data.teapot)
                                                                   .update(dt)
                                                                   .calc_quat();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);
        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }
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

        auto [s, va, vb, ib] = data.resource_manager.get_resources(data.s_h, data.va_h, data.vb_h, data.ib_h);

        s.bind();
        s.set_uniform("u_mvp", mvp);

        va.bind();
        vb.bind();
        vb.load_vertices(model.vertices);
        ib.bind();
        ib.load_indices(model.indices);
        glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, (void*)0);
    }

    void stop() {
    }
};

struct FontData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 1.0f, 0.0f, 1.0f, 1.0f };
    Media::Font font;
    Cameras::StationaryPerspective camera { glm::vec3(0, 0, -10), glm::normalize(glm::vec3(0, 0, 1)) };

    Renderer::ResourceManager resource_manager;
    Renderer::FontBatchRenderer font_batch_renderer;
};
struct FontLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        auto report_error = [](Utily::Error& error) {
            FAIL() << error.what();
        };

        auto ttf = std::move(Utily::FileReader::load_entire_file("assets/RobotoMono.ttf").on_error(report_error).value());

        Media::Font font;
        font.init(ttf).on_error(report_error);

        Media::FontAtlas font_atlas = std::move(font.gen_image_atlas(100).on_error(report_error).value());

        data.font_batch_renderer.init(data.resource_manager, font_atlas);

        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(float dt, const Core::InputManager& input, AppState& state, entt::registry& ecs, FontData& data) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);
        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }
    }
    void draw(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear(data.background_colour);
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Renderer::FontBatchRenderer::BatchConfig batch_config = {
            .resource_manager = data.resource_manager,
            .screen_dimensions = { renderer.window_width, renderer.window_height },
            .font_colour = { 0, 0, 0, 1 }
        };

        data.font_batch_renderer.begin_batch(std::move(batch_config));
        {
            data.font_batch_renderer.push_to_batch("hi there", { 0, 0 }, 80);
            for (int i = 0; i < renderer.window_height; i += 14) {
                data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah blah blah", { i / 14, i }, 25);
            }
        }
        data.font_batch_renderer.end_batch();
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
TEST(BasicApps, spinning_teapot) {
    auto_run_app<SpinningTeapotData, SpinningTeapotLogic>("Test App: Spinning Teapot");
}
TEST(BasicApps, font_rendering) {
    auto_run_app<FontData, FontLogic>("Test App: Font Rendering");
}

#elif defined(CONFIG_TARGET_WEB)

void run_font_renderer() {
    static App<FontData, FontLogic> st_app;
    st_app.init("font", 400, 400);
    emscripten_set_main_loop(
        []() {
            if (!st_app.is_running()) {
                EM_ASM_({
                    var filename = "FontAtlasGeneration.png";
                    var fileContents = FS.readFile(filename);
                    var blob = new Blob([fileContents], { type: 'application/octet-stream' });
                    var a = document.createElement('a');
                    document.body.appendChild(a);
                    a.href = URL.createObjectURL(blob);
                    a.download = filename;
                    a.click();
                    document.body.removeChild(a);
                    URL.revokeObjectURL(a.href);
                });
                Profiler::instance().save_as_trace_event_json(Profiler::TRACE_FILE_NAME);
                emscripten_cancel_main_loop();
            }
            st_app.poll_events();
            st_app.update();
            st_app.render();
        },
        0,
        1);
}

void run_spinning_teapot() {
    static App<SpinningTeapotData, SpinningTeapotLogic> st_app;
    st_app.init("spinner", 400, 400);
    emscripten_set_main_loop(
        []() {
            if (!st_app.is_running()) {
                emscripten_cancel_main_loop();
                run_font_renderer();
            }
            st_app.poll_events();
            st_app.update();
            st_app.render();
        },
        0,
        1);
}

void run_spinning_square() {
    static App<SpinningSquareData, SpinningSquareLogic> ss_app;
    ss_app.init("spinner", 400, 400);
    emscripten_set_main_loop(
        []() {
            if (!ss_app.is_running()) {
                emscripten_cancel_main_loop();
                run_spinning_teapot();
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