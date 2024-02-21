#pragma once

#include <chrono>
#include <cstdio>
#include <format>
#include <print>
#include <string_view>

#include <Utily/Utily.hpp>
#include <entt/entt.hpp>

#include "Cameras/Cameras.hpp"
#include "Renderer/Renderer.hpp"

#include "Io/Input.hpp"

#include "AppRenderer.hpp"

#include <chrono>
#include <thread>

struct AppState {
    bool should_close = false;
};

template <typename T, typename AppData>
concept HasValidAppLogic = requires(T t, double dt, AppState& state, AppData& data, AppRenderer& renderer, const Io::InputManager& input, entt::registry& ecs) {
    {
        t.init(renderer, ecs, data)
    } -> std::same_as<void>;
    {
        t.update(dt, input, state, ecs, data)
    } -> std::same_as<void>;
    {
        t.draw(renderer, ecs, data)
    } -> std::same_as<void>;
    {
        t.stop()
    } -> std::same_as<void>;
};

template <typename AppData, typename AppLogic>
    requires HasValidAppLogic<AppLogic, AppData>
class App
{
private:
    AppState _state;
    Io::InputManager _input;
    AppRenderer _renderer;
    entt::registry _ecs;
    AppData _data;
    AppLogic _logic;

    Renderer::OpenglContext _context;
    bool _has_init = false;
    std::chrono::high_resolution_clock::time_point _last_update;

public:
    auto init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> void {
        _context.init(app_name, width, height).on_error(Utily::ErrorHandler::print_then_quit);
        _ecs = entt::registry {};
        _logic.init(_renderer, _ecs, _data);
        _input.init(_context.unsafe_window_handle());
        _has_init = true;
    }
    auto stop() -> void {
        _renderer.stop();
        _logic.stop();
        _context.stop();
    }
    auto update() -> void {
        double dt = std::chrono::duration<double> { std::chrono::high_resolution_clock::now() - _last_update }.count();
        _logic.update(dt, _input, _state, _ecs, _data);
        _last_update = std::chrono::high_resolution_clock::now();
    }
    auto render() -> void {
        _renderer.window_width = _context.window_width;
        _renderer.window_height = _context.window_height;
        _logic.draw(_renderer, _ecs, _data);
        _context.swap_buffers();
    }

    auto poll_events() -> void {
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
    while (app.is_running()) {
        app.poll_events();
        app.update();
        app.render();
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
