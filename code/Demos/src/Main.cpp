#include <chrono>
#include <iostream>
#include <span>
#include <string_view>

#include <Engine.hpp>
#include <Renderer/Renderer.hpp>
#include <Utily/Utily.hpp>

using namespace std::literals;

static inline auto print_then_quit = [](const auto& error) {
    std::cout << error.what();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    throw std::runtime_error(std::string(error.what()));
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
#elif 0

struct FontData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 1.0f, 0, 1.0f, 1.0f };

    Media::Font font {};
    Media::FontAtlas font_atlas {};

    Renderer::ResourceManager resource_manager {};
    Renderer::FontRenderer font_renderer {};
    Renderer::FontBatchRenderer font_batch_renderer {};
};
struct FontLogic {
    void init(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        data.start_time = std::chrono::high_resolution_clock::now();
        auto ttf_raw = Utily::FileReader::load_entire_file("assets/RobotoMono.ttf").on_error(print_then_quit);

        auto e = data.font.init(ttf_raw.value()).on_error(print_then_quit);

        data.font_atlas.init(data.font, 100).on_error(print_then_quit);
        data.font_atlas.image.save_to_disk("FontAtlasGeneration.png").on_error(print_then_quit);
        data.font_renderer.init(data.resource_manager, data.font_atlas);
        data.font_batch_renderer.init(data.resource_manager, data.font_atlas);
    }
    void update(float dt, const Core::InputManager& input, AppState& state, entt::registry& ecs, FontData& data) {
    }
    void draw(AppRenderer& renderer, entt::registry& ecs, FontData& data) {
        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear(data.background_colour);
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        Renderer::FontBatchRenderer::BatchConfig config {
            .resource_manager = data.resource_manager,
            .screen_dimensions = glm::vec2 { renderer.window_width, renderer.window_height },
            .font_colour = { 0, 0, 0, 1 },
        };

        data.font_batch_renderer.begin_batch(std::move(config));
        {
            data.font_batch_renderer.push_to_batch("hi there", { 0, 0 }, 100);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. lah blah blah", { 0, 25 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blh blah blah", { 0, 50 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. bah blah blah", { 0, 75 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah bah blah", { 0, 100 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah blah blah", { 0, 125 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah blah bah", { 0, 150 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah blah bla", { 0, 175 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. lah blah blah", { 0, 40 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blh blah blah", { 0, 60 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. bah blah blah", { 0, 80 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah bah blah", { 0, 110 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah blah blah", { 0, 130 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah blah bah", { 0, 160 }, 25);
            data.font_batch_renderer.push_to_batch("this is a lot of text on the screen. blah blah bla", { 0, 180 }, 25);
        }
        data.font_batch_renderer.end_batch();
    }
    void stop() {
    }
};

#else
struct IsoData {
    std::chrono::steady_clock::time_point start_time;
    glm::vec4 background_colour = { 1, 1, 0, 1 };

    Core::AudioManager::BufferHandle sound_buffer;
    Core::AudioManager::SourceHandle source_handle;

    Components::Spinning spinning;

    Renderer::ResourceManager resource_manager;
    Renderer::InstanceRenderer instance_renderer;

    std::optional<Renderer::FontBatchRenderer> font_batch_renderer;

    Cameras::StationaryPerspective camera { glm::vec3(0, 1, -1), glm::normalize(glm::vec3(0, -0.25f, 0.5f)) };
};
struct IsoLogic {
    void init(AppRenderer& renderer, Core::AudioManager& audio, IsoData& data) {

        Media::Sound sound = Media::Sound::create("assets/background_sound.wav").on_error_panic().value_move();

        auto scheduler = std::move(Core::Scheduler::create(2).value());

        scheduler.add_task([]() {
            Media::FontAtlas::create("assets/RobotoMono.ttf", 500).on_error_panic().value().atlas_image().save_to_disk("RobotoMonoAtlas.png");
        });

        std::mutex sound_mutex;
        std::optional<Media::Sound> sound_2;
        scheduler.add_task([&]() {
            auto create_sound_result = Media::Sound::create("assets/background_sound.wav");

            sound_mutex.lock();
            sound_2.emplace(create_sound_result.on_error_panic().value_move());
            sound_mutex.unlock();
        });

        std::mutex model_mutex;
        std::optional<Model::Static> model_data_2;
        scheduler.add_task([&]() {
            auto model_data = Utily::FileReader::load_entire_file("assets/teapot.obj").on_error_panic().value_move();
            auto model_decode_result = Model::decode_as_static_model(model_data, ".obj");

            model_mutex.lock();
            model_data_2.emplace(model_decode_result.on_error_panic().value_move());
            model_mutex.unlock();
        });

        std::mutex image_mutex;
        std::optional<Media::Image> image_2;
        scheduler.add_task([&]() {
            auto image_result = Media::Image::create("assets/texture.png");

            image_mutex.lock();
            image_2.emplace(image_result.on_error_panic().value_move());
            image_mutex.unlock();
        });

        scheduler.launch_threads();

        auto res = audio.load_sound_into_buffer(sound).on_error(print_then_quit);

        data.sound_buffer = res.value();

        data.spinning.axis_of_rotation = { 0, 1, 0 };
        data.spinning.angle = 0;
        data.spinning.rotations_per_second = 1;

        auto model_data = Utily::FileReader::load_entire_file("assets/teapot.obj").on_error_panic().value_move();
        auto model = Model::decode_as_static_model(model_data, ".obj").on_error_panic().value_move();
        auto image = Media::Image::create("assets/texture.png").on_error_panic().value_move();

        data.font_batch_renderer.emplace(Renderer::FontBatchRenderer::create(data.resource_manager, "assets/RobotoMono.ttf")
                                             .on_error_panic()
                                             .value_move());

        data.instance_renderer.init(data.resource_manager, model, image);
        data.source_handle = audio.play_sound(data.sound_buffer, { 5, 0, 0 }).on_error(print_then_quit).value();

        scheduler.wait_for_threads();

        data.start_time = std::chrono::high_resolution_clock::now();
    }

    void update(float dt, const Core::InputManager& input, Core::AudioManager& audio, AppState& state, IsoData& data) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - data.start_time);

        if (duration > std::chrono::seconds(3)) {
            state.should_close = true;
        }

        data.spinning.update(dt);
        auto quat = data.spinning.calc_quat();
        glm::vec4 point { 5, 0, 0, 1 };
        point = quat * point;

        audio.set_source_motion(data.source_handle, point, { 0, 0, 0 }).on_error(print_then_quit);

        Core::AudioManager::ListenerProperties lp {};
        lp.pos = { 0, 0, 0 };
        lp.dir = { 0, 0, 1 };
        lp.vel = { 0, 0, 0 };
        audio.set_listener_properties(lp);

        auto t = Components::Transform {};
        t.position = glm::vec3(0, -1, 1);
        t.scale = glm::vec3(0.5f);

        auto model = t.calc_transform_mat();
        data.instance_renderer.push_instance(model);

        t.position = glm::vec3(0, -1, 2);
        model = t.calc_transform_mat();
        data.instance_renderer.push_instance(model);
    }

    void draw(AppRenderer& renderer, IsoData& data) {
        renderer.screen_frame_buffer.bind();
        renderer.screen_frame_buffer.clear(data.background_colour);
        renderer.screen_frame_buffer.resize(renderer.window_width, renderer.window_height);

        auto v = data.camera.view_matrix();
        auto p = data.camera.projection_matrix(renderer.window_width, renderer.window_height);
        data.instance_renderer.draw_instances(data.resource_manager, v, p);
    }
    void stop(IsoData& data) {
    }
};

#endif

int main() {
    auto_run_app<IsoData, IsoLogic>("Demo", 1000, 500);
    return 0;
}