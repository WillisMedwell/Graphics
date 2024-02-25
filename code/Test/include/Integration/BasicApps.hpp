#pragma once

#include "TestPch.hpp"

#include <iostream>
#include <thread>

#include <App.hpp>
#include <AppAnalytics.hpp>
#include <Components/Components.hpp>
#include <Model/Model.hpp>

using namespace std::literals;

struct BackgroundColourData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 0, 0, 0, 1.0f };
};
struct BackgroundColourLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, BackgroundColourData& data) {
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
    void update(float dt, const Io::InputManager& input, AppState& state, entt::registry& ecs, BackgroundColourData& data) {

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

        renderer.add_vertex_array(Renderer::VertexBufferLayout<glm::vec3> {}, data.vb_id)
            .on_error(Utily::ErrorHandler::print_then_quit)
            .on_value([&](auto& id) {
                data.va_id = id;
            });
    }
    void update(double dt, const Io::InputManager& input, AppState& state, entt::registry& ecs, SpinningSquareData& data) {

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

struct SpinningTeapotData {
    std::chrono::steady_clock::time_point start_time;
    entt::entity teapot;
    Cameras::StationaryPerspective camera { glm::vec3(0, 1, -1), glm::normalize(glm::vec3(0, -0.25f, 0.5f)) };

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
        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(double dt, const Io::InputManager& input, AppState& state, entt::registry& ecs, SpinningTeapotData& data) {
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
    }

    void stop() {
    }
};

struct FontData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 0.25f, 0.25f, 0.0f, 1.0f };
    Media::Font font;
    Cameras::StationaryPerspective camera { glm::vec3(0, 0, -10), glm::normalize(glm::vec3(0, 0, 1)) };

    AppRenderer::ShaderId s_id;
    AppRenderer::IndexBufferId ib_id;
    AppRenderer::VertexBufferId vb_id;
    AppRenderer::VertexArrayId va_id;
    AppRenderer::TextureId t_id;

    constexpr static std::string_view VERT =
        "precision highp float; "
        "uniform mat4 u_mvp;"
        "layout(location = 0) in vec2 l_pos;"
        "layout(location = 1) in vec2 l_uv;"
        "out vec2 uv;"
        "void main() {"
        "    gl_Position = u_mvp * vec4(l_pos, -1.0, 1.0);"
        "    uv = l_uv;"
        "}"sv;
    constexpr static std::string_view FRAG =
        "precision highp float; "
        "uniform sampler2D u_texture;"
        "out vec4 FragColor;"
        "in vec2 uv;"
        "void main() {"
        "    vec2 uv_flipped = vec2(uv.x, 1.0f - uv.y);"
        "    FragColor = vec4(texture(u_texture, uv_flipped).rrrr);"
        "}"sv;

    Media::FontAtlas font_atlas = {};
};
struct FontLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        auto report_error = [](Utily::Error& error) {
            FAIL() << error.what();
        };

        auto ttf_raw = Utily::FileReader::load_entire_file("assets/RobotoMono.ttf").on_error(report_error).value();

        data.font.init(ttf_raw);
        data.font_atlas.init(data.font, 100).on_error(report_error);
        data.font_atlas.image.save_to_disk("FontAtlasGeneration.png").on_error(report_error);

        data.s_id = renderer.add_shader(data.VERT, data.FRAG).on_error(report_error).value();
        data.ib_id = renderer.add_index_buffer().on_error(report_error).value();
        data.vb_id = renderer.add_vertex_buffer().on_error(report_error).value();
        data.va_id = renderer.add_vertex_array(Model::Vertex2D::VBL {}, data.vb_id).on_error(report_error).value();
        data.t_id = renderer.add_texture(data.font_atlas.image).on_error(report_error).value();

        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(float dt, const Io::InputManager& input, AppState& state, entt::registry& ecs, FontData& data) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);
        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }
    }
    void draw(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        renderer.screen_frame_buffer.clear(data.background_colour);

        auto pm = Cameras::Orthographic::projection_matrix(renderer.window_width, renderer.window_height);

        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Renderer::IndexBuffer& ib = renderer.index_buffers[data.ib_id.id];
        Renderer::VertexBuffer& vb = renderer.vertex_buffers[data.vb_id.id];
        Renderer::VertexArray& va = renderer.vertex_arrays[data.va_id.id];
        Renderer::Shader& s = renderer.shaders[data.s_id.id];
        Renderer::Texture& t = renderer.textures[data.t_id.id];

        const static auto [verts, indis] = Media::FontMeshGenerator::generate_static_mesh("hello there", 100, { 50, 50 }, data.font_atlas);

        s.bind();

        s.set_uniform("u_mvp", pm);
        s.set_uniform("u_texture", static_cast<int>(t.bind().value()));

        va.bind();
        ib.bind();
        vb.bind();
        ib.load_indices(indis);
        vb.load_vertices(verts);
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