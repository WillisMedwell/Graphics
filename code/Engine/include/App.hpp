#pragma once

#include <chrono>
#include <cstdio>
#include <format>
#include <print>
#include <string_view>

#include "Util/Concepts.hpp"
#include "Util/ErrorHandling.hpp"

#include "Renderer/OpenglContext.hpp"

struct AppData {
};

template <typename T>
concept HasAppLogic = requires(T t, float dt, AppData& data) {
    {
        t.init(data)
    } -> std::same_as<void>;
    {
        t.update(dt, data)
    } -> std::same_as<void>;
    {
        t.stop()
    } -> std::same_as<void>;
    {
        t.NAME
    } -> std::convertible_to<std::string_view>;
};

template <HasAppLogic AppLogic>
class App
{
private:
    AppLogic _logic;
    AppData _data;

    Renderer::OpenglContext _context;

    bool _has_init = false;
    bool _must_stop = false;

    std::chrono::high_resolution_clock::time_point _last_update;

public:
    auto init() -> void {
        _context.init(_logic.NAME, 1000, 1000).on_error(Util::ErrorHandling::print);

        _logic.init(_data);
        _has_init = true;
        std::println("The App \"{}\" has successfully initialised.", _logic.NAME);
    }
    auto stop() -> void {
        _logic.stop();
        std::println("The App \"{}\" has stopped.", _logic.NAME);
    }
    auto update() -> void {
        float dt = std::chrono::duration<float> { std::chrono::high_resolution_clock::now() - _last_update }.count();
        _logic.update(dt, _data);
        _last_update = std::chrono::high_resolution_clock::now();
    }
    auto render() -> void {
    }

    auto poll_inputs() -> void {
    }
    auto is_running() -> bool {
        return (!_has_init) || _must_stop;
    }
    ~App() {
        stop();
    }
};

/*
// The basic App-Logic System is shown below:

struct Logic {
    void init(AppData& data) {
    }
    void update(float dt, AppData& data) {
    }
    void stop() {
    }
    constexpr static auto NAME = std::string_view { "App-Name" };
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