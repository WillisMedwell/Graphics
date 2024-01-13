#pragma once

#include <chrono>
#include <cstdio>
#include <format>
#include <print>
#include <string_view>

#include <entt/entt.hpp>

#include "Util/Util.hpp"
#include "Renderer/Renderer.hpp"

struct AppData {
    bool should_close = false;
    entt::registry ecs;
};

struct AppInput {
    
};

template <typename T>
concept HasAppLogic = requires(T t, float dt, AppData& data, const AppInput& input) {
    {
        t.init(data)
    } -> std::same_as<void>;
    {
        t.update(dt, data, input)
    } -> std::same_as<void>;
    {
        t.stop()
    } -> std::same_as<void>;
};

template <HasAppLogic AppLogic>
class App
{
private:
    AppLogic _logic;
    AppData _data;
    AppInput _input;

    Renderer::OpenglContext _context;

    bool _has_init = false;

    std::chrono::high_resolution_clock::time_point _last_update;

public:
    auto init(std::string_view app_name, uint_fast16_t width, uint_fast16_t height) -> void {
        _context.init(app_name, width, height).on_error(Util::ErrorHandling::print);
        _logic.init(_data);
        _has_init = true;
    }
    auto stop() -> void {
        _logic.stop();
    }
    auto update() -> void {
        float dt = std::chrono::duration<float> { std::chrono::high_resolution_clock::now() - _last_update }.count();
        _logic.update(dt, _data, _input);
        _last_update = std::chrono::high_resolution_clock::now();
    }
    auto render() -> void {
    }

    auto poll_events() -> void {
        this->_context.poll_events();
    }
    auto is_running() -> bool {
        return _has_init && !_context.should_close() && !_data.should_close;
    }
    auto request_fullscreen() {

    }

    ~App() {
        stop();
    }
};

template<HasAppLogic Logic>
void autoRunApp()
{
    App<Logic> app;
    app.init("AutoApp", 400, 400);
    while (app.is_running()) {
        app.poll_events();
        app.update();
        app.render();
    }
    app.stop();
}

/*
// The basic App-Logic System is shown below:

struct Logic {
    void init(AppData& data) {
    }
    void update(float dt, AppData& data, const AppInput& input) {
    }
    void stop() {
    }
};

void main() {
    App<Logic> app;

    app.init();
    while (app.is_running()) {
        app.poll_inputs();
        app.update();
        app.render();
    }
    app.stop();
}
*/