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

#include "AppInput.hpp"
#include "AppRenderer.hpp"

struct AppState {
    bool should_close = false;
};

template <typename T, typename AppData>
concept HasValidAppLogic = requires(T t, float dt, AppState& state, AppData& data, AppRenderer& renderer, AppInput& input) {
    {
        t.init(data)
    } -> std::same_as<void>;
    {
        t.update(dt, input, state, data)
    } -> std::same_as<void>;
    {
        t.draw(renderer, data)
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
    AppInput _input;
    AppRenderer _renderer;

    AppData _data;
    AppLogic _logic;

    Renderer::OpenglContext _context;
    bool _has_init = false;
    std::chrono::high_resolution_clock::time_point _last_update;

public:
    auto init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> void {
        _context.init(app_name, width, height).on_error(Utily::ErrorHandler::print_then_quit);
        _logic.init(_data);
        _has_init = true;
    }
    auto stop() -> void {
        _logic.stop();
    }
    auto update() -> void {
        float dt = std::chrono::duration<float> { std::chrono::high_resolution_clock::now() - _last_update }.count();
        _logic.update(dt, _input, _state, _data);
        _last_update = std::chrono::high_resolution_clock::now();
    }
    auto render() -> void {
        _renderer.window_width = _context.window_width;
        _renderer.window_height = _context.window_height;
        _logic.draw(_renderer, _data);
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
void autoRunApp() {
    App<Data, Logic> app;
    app.init("AutoApp", 400, 400);
    while (app.is_running()) {
        app.poll_events();
        app.update();
        app.render();
    }
    app.stop();
}