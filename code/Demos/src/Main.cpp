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

static inline auto print_then_quit = [](const auto& error) {
    std::cout << error.what();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    exit(EXIT_FAILURE);
};

#if 0
class TimingBomb
{
public:
    // Constructor
    TimingBomb(const std::string& message)
        : _message(message)
        , _start(std::chrono::steady_clock::now()) { }

    // Destructor
    ~TimingBomb() {
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - _start;
        std::cout << _message << ": " << elapsed_seconds.count() * 1000 << "ms\n";
    }

private:
    std::string _message;
    std::chrono::time_point<std::chrono::steady_clock> _start;
};

#if 0
struct SpinningTeapotData {
    entt::entity teapot;

    entt::entity ground;
    entt::entity cube;

    Cameras::StationaryPerspective camera { glm::vec3(0, 10, 0), glm::normalize(glm::vec3(0, -1, 0)) };
    AppRenderer::ShaderId s_id;
    AppRenderer::IndexBufferId ib_id;
    AppRenderer::VertexArrayId va_id;
    AppRenderer::VertexBufferId vb_id;

    constexpr static std::string_view VERT =
        "precision highp float; "
        "uniform mat4 u_mvp;"
        "layout(location = 0) in vec3 aPos;"
        "layout(location = 1) in vec3 aNor;"
        "layout(location = 2) in vec2 aUv;"
        "void main() {"
        "    gl_Position = u_mvp * vec4(aPos, 1.0);"
        "}"sv;
    constexpr static std::string_view FRAG =
        "precision highp float; "
        "uniform vec3 u_colour;"
        "out vec4 FragColor;"
        "void main() {"
        "    FragColor = vec4(u_colour, 1.0);"
        "}"sv;
};
struct SpinningTeapotLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, SpinningTeapotData& data) {

        TimingBomb tb("Spinning teapot logic init");
        auto teapot_source = Utily::FileReader::load_entire_file("assets/teapot.obj")
                                 .on_error(print_then_quit)
                                 .value();

        auto teapot_model = std::move(Model::decode_as_static_model(teapot_source, { '.', 'o', 'b', 'j' })
                                          .on_error(print_then_quit)
                                          .value());

        auto ground_model = Model::generate_plane(
            glm::vec3 { 0, 0, 0 },
            glm::vec3 { 1, 0, 0 },
            glm::vec3 { 1, 0, 1 },
            glm::vec3 { 0, 0, 1 });

        data.teapot = ecs.create();
        ecs.emplace<Model::Static>(data.teapot, std::move(ground_model));
        // ecs.emplace<Components::Transform>(data.teapot, glm::vec3 { 0, -1, 0 }, glm::vec3 { 1.0f });
        // ecs.emplace<Components::Spinning>(data.teapot, glm::vec3 { 0, 1, 0 }, 0.0, 0.5);

        renderer.add_shader(data.VERT, data.FRAG)
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.s_id = id;
            });
        renderer.add_vertex_buffer()
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.vb_id = id;
            });
        renderer.add_index_buffer()
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.ib_id = id;
            });
        renderer.add_vertex_array(Model::Vertex::VBL {}, data.vb_id)
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.va_id = id;
            });
    }
    void update(double dt, const Io::InputManager& input, AppState& state, entt::registry& ecs, SpinningTeapotData& data) {
        /*
        auto& transform = ecs.get<Components::Transform>(data.teapot);

        transform.rotation = ecs.get<Components::Spinning>(data.teapot).update(dt).calc_quat();

        if (auto w = input.get_key_state(Io::Inputs::Keyboard::w);
            w == Io::InputState::Keyboard::held || w == Io::InputState::Keyboard::pressed) {
            transform.position.z += dt * 1.0f;
            std::cout << "w\n";
        }
        if (auto s = input.get_key_state(Io::Inputs::Keyboard::s);
            s == Io::InputState::Keyboard::held || s == Io::InputState::Keyboard::pressed) {
            transform.position.z -= dt * 1.0f;
            std::cout << "s\n";
        }


        if (auto state = input.get_key_state(Io::Inputs::Keyboard::w); state == Io::InputState::Keyboard::pressed || state == Io::InputState::Keyboard::held) {
            std::cout << "w\n";
        }
        auto mouse_pos = input.get_mouse_state().position;
        std::cout << mouse_pos.x << ' ' << mouse_pos.y << '\n';
        if (auto state = input.get_mouse_state().button_left; state == Io::InputState::Keyboard::pressed || state == Io::InputState::Keyboard::held) {
            std::cout << "mouse down\n";
        }


        std::cout << transform.position.x << " " << transform.position.y << " " << transform.position.z << '\n';
        */
    }

    void draw(AppRenderer& renderer, entt::registry& ecs, SpinningTeapotData& data) {
        auto& model = ecs.get<Model::Static>(data.teapot);

        // transform.rotation = spinning.calc_quat();
        auto pm = data.camera.projection_matrix(renderer.window_width, renderer.window_height);
        auto vm = data.camera.view_matrix();
        // auto mm = transform.calc_transform_mat();

        auto mvp = pm * vm; //* mm;

        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear();
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Renderer::IndexBuffer& ib = renderer.index_buffers[data.ib_id.id];
        Renderer::VertexBuffer& vb = renderer.vertex_buffers[data.vb_id.id];
        Renderer::VertexArray& va = renderer.vertex_arrays[data.va_id.id];
        Renderer::Shader& s = renderer.shaders[data.s_id.id];

        s.bind();

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
#endif

struct SpinningTeapotData {
    std::chrono::steady_clock::time_point start_time;
    entt::entity teapot;
    entt::entity ground_plane;

    Cameras::StationaryPerspective camera { glm::vec3(0, 0, -10), glm::normalize(glm::vec3(0, 0, 1)) };

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
        "void main() {"
        "    gl_Position = u_mvp * vec4(aPos, 1.0);"
        "}"sv;
    constexpr static std::string_view FRAG =
        "precision highp float; "
        "uniform vec3 u_colour;"
        "out vec4 FragColor;"
        "void main() {"
        "    FragColor = vec4(u_colour, 1.0);"
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
        ecs.emplace<Components::Transform>(data.teapot, glm::vec3 { 0, 0, 0 }, glm::vec3(1.0f));
        ecs.emplace<Components::Spinning>(data.teapot, glm::vec3 { 0, 1, 0 }, 0.0, 1.0);

       /* Model::generate_plane({ 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 };*/
        data.ground_plane = ecs.create();
        ecs.emplace<Model::Static>(data.ground_plane,Model::generate_axis());
        ecs.emplace<Components::Transform>(data.ground_plane, glm::vec3 { 0, 0, 0 }, glm::vec3(10.0f));

        renderer.add_shader(data.VERT, data.FRAG)
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.s_id = id;
            });
        renderer.add_vertex_buffer()
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.vb_id = id;
            });
        renderer.add_index_buffer()
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.ib_id = id;
            });
        renderer.add_vertex_array(Model::Vertex::VBL {}, data.vb_id)
            .on_error(print_then_quit)
            .on_value([&](auto& id) {
                data.va_id = id;
            });
        data.start_time = std::chrono::high_resolution_clock::now();
    }
    void update(double dt, const Io::InputManager& input, AppState& state, entt::registry& ecs, SpinningTeapotData& data) {
        // TimingBomb a("update");
        ecs.get<Components::Transform>(data.teapot).rotation = ecs.get<Components::Spinning>(data.teapot)
                                                                   .update(dt)
                                                                   .calc_quat();

        /*auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);
        if (duration > std::chrono::seconds(1)) {
            state.should_close = true;
        }*/

        if (auto w = input.get_key_state(Io::Inputs::Keyboard::w);
            w == Io::InputState::Keyboard::held || w == Io::InputState::Keyboard::pressed) {
            
            data.camera.increment_direction_via_angles(dt * 10, 0);
            std::cout << data.camera.direction.z << "\n";
        }
        if (auto s = input.get_key_state(Io::Inputs::Keyboard::s);
            s == Io::InputState::Keyboard::held || s == Io::InputState::Keyboard::pressed) {
            
            data.camera.increment_direction_via_angles(dt * -10, 0);

            std::cout << data.camera.direction.z << "\n";

            auto normalised = glm::normalize(data.camera.direction);

            std::cout << data.camera.direction.x << ' ' << data.camera.direction.y << ' ' << data.camera.direction.z << '\n';
        }
    }

    void draw(AppRenderer& renderer, entt::registry& ecs, SpinningTeapotData& data) {

        // TimingBomb a("draw");

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
        s.set_uniform("u_mvp", mvp)
            .on_error(print_then_quit);
        s.set_uniform("u_colour", { 222.0f / 255.0f, 0.0f, 1.0f })
            .on_error(print_then_quit);
        va.bind();
        ib.bind();
        vb.bind();
        ib.load_indices(model.indices);
        vb.load_vertices(model.vertices);
        glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, (void*)0);

        auto [ground_model, ground_transform] = ecs.get<Model::Static, Components::Transform>(data.ground_plane);

        mm = ground_transform.calc_transform_mat();
        mvp = pm * vm * mm;
        s.set_uniform("u_mvp", mvp)
            .on_error(print_then_quit);
        s.set_uniform("u_colour", { 0.2f, 0.2f, 0.2f })
            .on_error(print_then_quit);

        ib.load_indices(ground_model.indices);
        vb.load_vertices(ground_model.vertices);
        glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, (void*)0);
    }

    void stop() {
    }
};
#endif

struct FontData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 0, 0, 0, 1.0f };
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
        "layout(location = 0) in vec3 aPos;"
        "layout(location = 1) in vec3 aNor;"
        "layout(location = 2) in vec2 aUv;"
        "out vec3 pos;"
        "out vec3 nor;"
        "out vec2 uv;"
        "void main() {"
        "    gl_Position = u_mvp * vec4(aPos, 1.0);"
        "    pos = aPos;"
        "    nor = aNor;"
        "    uv = aUv;"
        "}"sv;
    constexpr static std::string_view FRAG =
        "precision highp float; "
        "uniform sampler2D u_texture;"
        "out vec4 FragColor;"
        "in vec2 uv;"
        "void main() {"
        "    vec2 uv_flipped = vec2(uv.x, 1 - uv.y);"
        "    FragColor = vec4(texture(u_texture, uv_flipped).rrr, 1);"
        "}"sv;

    Media::FontAtlas font_atlas = {};
};
struct FontLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        data.start_time = std::chrono::high_resolution_clock::now();
        auto ttf_raw = Utily::FileReader::load_entire_file("assets/RobotoMono.ttf").on_error(print_then_quit);

        auto e = data.font.init(ttf_raw.value()).on_error(print_then_quit);

        data.font_atlas.init(data.font, 100).on_error(print_then_quit);
        data.font_atlas.image.save_to_disk("FontAtlasGeneration.png").on_error(print_then_quit);

        data.s_id = renderer.add_shader(data.VERT, data.FRAG).on_error(print_then_quit).value();
        data.ib_id = renderer.add_index_buffer().on_error(print_then_quit).value();
        data.vb_id = renderer.add_vertex_buffer().on_error(print_then_quit).value();
        data.va_id = renderer.add_vertex_array(Model::Vertex::VBL {}, data.vb_id).on_error(print_then_quit).value();
        data.t_id = renderer.add_texture(data.font_atlas.image).on_error(print_then_quit).value();
    }
    void update(float dt, const Io::InputManager& input, AppState& state, entt::registry& ecs, FontData& data) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);
        if (duration > std::chrono::seconds(1)) {
            // state.should_close = true;
        }
    }
    void draw(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        renderer.screen_frame_buffer.clear(data.background_colour);

        // auto pm = data.camera.projection_matrix(renderer.window_width, renderer.window_height);
        auto pm = Cameras::Orthographic::projection_matrix(renderer.window_width, renderer.window_height);

        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear();
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Renderer::IndexBuffer& ib = renderer.index_buffers[data.ib_id.id];
        Renderer::VertexBuffer& vb = renderer.vertex_buffers[data.vb_id.id];
        Renderer::VertexArray& va = renderer.vertex_arrays[data.va_id.id];
        Renderer::Shader& s = renderer.shaders[data.s_id.id];
        Renderer::Texture& t = renderer.textures[data.t_id.id];

        static auto str_model = Media::FontMeshGenerator::generate_static_mesh("hello", 100, { 50, 50 }, data.font_atlas);

        s.bind();

        s.set_uniform("u_mvp", pm).on_error(print_then_quit);
        s.set_uniform("u_texture", static_cast<int>(t.bind().value())).on_error(print_then_quit);

        va.bind();
        ib.bind();
        vb.bind();
        ib.load_indices(str_model.indices);
        vb.load_vertices(str_model.vertices);
        glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, (void*)0);
    }
    void stop() {
    }
};

int main() {
    auto_run_app<FontData, FontLogic>("Demo", 1000, 500);
    return 0;
}