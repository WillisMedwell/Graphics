#pragma once

#include <chrono>
#include <cstdio>
#include <format>
#include <print>
#include <string_view>

#include <Utily/Utily.hpp>
#include <entt/entt.hpp>

#include "Cameras/Cameras.hpp"
#include "Core/Core.hpp"
#include "Profiler/Profiler.hpp"

#include "Core/Input.hpp"

#include "App/AppRenderer.hpp"

#include <chrono>
#include <thread>

struct AppState {
    bool should_close = false;
};

template <typename T, typename AppData>
concept HasValidAppLogic = requires(T t, double dt, AppState& state, AppData& data, AppRenderer& renderer, Core::AudioManager& audio, const Core::InputManager& input) {
    {
        t.init(renderer, audio, data)
    } -> std::same_as<void>;
    {
        t.update(dt, input, audio, state, data)
    } -> std::same_as<void>;
    {
        t.draw(renderer, data)
    } -> std::same_as<void>;
    {
        t.stop(data)
    } -> std::same_as<void>;
};

template <typename AppData, typename AppLogic>
    requires HasValidAppLogic<AppLogic, AppData>
class App
{
private:
    AppState _state;
    Core::InputManager _input;
    AppRenderer _renderer;
    entt::registry _ecs;
    AppData _data;
    AppLogic _logic;

    Core::AudioManager _audio;

    Core::OpenglContext _context;
    bool _has_init = false;
    bool _has_stopped = false;
    std::chrono::high_resolution_clock::time_point _last_update;

    inline static auto panic = [](auto& error) {
        std::cerr << error.what() << std::endl;
        throw std::runtime_error(std::string(error.what()));
    };

public:
    auto init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> void {
        Profiler::instance().switch_to_process(Utily::Reflection::get_type_name<AppLogic>());
        Profiler::Timer timer("App::init()", { "App" });

        _context.init(app_name, width, height).on_error(Utily::ErrorHandler::print_then_quit);
        _input.init(_context.unsafe_window_handle());

        _audio.init().on_error(panic);
        _ecs = entt::registry {};

        {
            Profiler::Timer timer2("Logic::init()");
            _logic.init(_renderer, _audio, _data);
        }

        _has_init = true;
        _has_stopped = false;
    }
    auto stop() -> void {
        if (!_has_stopped) {
            Profiler::Timer timer("App::stop()", { "App" });
            _logic.stop(_data);
            _renderer.stop();
            _audio.stop();
            _context.stop();
        }
        _has_stopped = true;
    }
    auto update() -> void {
        Profiler::Timer timer("App::update()", { "App" });
        double dt = std::chrono::duration<double> { std::chrono::high_resolution_clock::now() - _last_update }.count();
        {
            Profiler::Timer timer2("Logic::update()");
            _logic.update(dt, _input, _audio, _state, _data);
        }
        _last_update = std::chrono::high_resolution_clock::now();
    }
    auto render() -> void {
        Profiler::Timer timer("App::render()", { "App" });
        _renderer.window_width = _context.window_width;
        _renderer.window_height = _context.window_height;

        {
            Profiler::Timer timer2("Logic::draw()");
            _logic.draw(_renderer, _data);
        }
        _context.swap_buffers();
    }

    auto poll_events() -> void {
        Profiler::Timer timer("App::poll_events()", { "App" });
        this->_context.poll_events();
    }
    auto is_running() -> bool {
        return _has_init && !_context.should_close() && !_state.should_close;
    }

    ~App() {
        stop();
    }
};

template <typename Data, typename Logic>
void auto_run_app(std::string_view app_name = "Auto Running App", uint16_t width = 400, uint16_t height = 400) {
    static App<Data, Logic> app;
    app.init(app_name, width, height);

#if defined(CONFIG_TARGET_NATIVE)
    {
        while (app.is_running()) {
            Profiler::Timer timer("App::main_loop()");
            app.poll_events();
            app.update();
            app.render();
        }
    }
    app.stop();
#elif defined(CONFIG_TARGET_WEB)
    emscripten_set_main_loop(
        []() {
            if (!app.is_running()) {
                emscripten_cancel_main_loop();
            }
            app.poll_events();
            app.update();
            app.render();
        },
        0,
        0);
#endif
}
