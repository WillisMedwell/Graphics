#pragma once

#include <Utily/Utily.hpp>
#include <map>
#include <source_location>
#include <thread>
#include <vector>

class Profiler
{
private:
    struct Recording {
        std::string_view name;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
        std::thread::id thread_id;
        std::vector<std::string_view> categories;
    };
    std::mutex _profiler_mutex;
    std::string_view _current_process;
    std::chrono::time_point<std::chrono::high_resolution_clock> _profiler_start_time;
    std::unordered_map<std::string_view, std::vector<Recording>> _processes_recordings;

public:
    class Timer
    {
    public:
        std::string_view name;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
        std::thread::id thread_id;
        std::vector<std::string_view> categories;

        Timer() = delete;
        Timer(const Timer&) = delete;
        Timer(Timer&&) = delete;

        Timer(std::string_view function_name = std::source_location::current().function_name(), std::vector<std::string_view> cats = {});
        ~Timer();
    };

    static auto instance() -> Profiler&;

    void switch_to_process(std::string_view process);
    void submit_timer(const Timer& timer);
    auto format_as_trace_event_json() -> std::string;
    auto save_as_trace_event_json(std::filesystem::path path) -> Utily::Result<void, Utily::Error>;

    constexpr static std::string_view TRACE_FILE_NAME = "app_trace.json";

private:
    Profiler()
        : _profiler_mutex()
        , _current_process("default")
        , _profiler_start_time(std::chrono::high_resolution_clock::now())
        , _processes_recordings({}) { }

    Profiler(const Profiler&) = delete;
    Profiler(Profiler&&) = delete;

public:
    ~Profiler();
};