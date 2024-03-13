#include "Profiler/Profiler.hpp"
#include "Config.hpp"

#include <map>
#include <mutex>
#include <string>

auto Profiler::instance() -> Profiler& {
    static Profiler profiler = {};
    return profiler;
}

void Profiler::switch_to_process(std::string_view process) {
    if constexpr (Config::SKIP_PROFILE) {
        return;
    }
    _current_process = process;
}
void Profiler::submit_timer(const Timer& timer) {
    if constexpr (Config::SKIP_PROFILE) {
        return;
    }
    _profiler_mutex.lock();
    auto& recordings = _processes_recordings[_current_process];
    recordings.emplace_back(timer.name, timer.start_time, timer.end_time, timer.thread_id, timer.categories);
    _profiler_mutex.unlock();
}
auto Profiler::format_as_trace_event_json() -> std::string {
    if constexpr (Config::SKIP_PROFILE) {
        return {};
    }

    std::scoped_lock lock(_profiler_mutex);

    std::string res;

    res += "{ \n\t\"traceEvents\": [ \n";

    std::unordered_map<std::thread::id, int> thread_names {};

    for (const auto& [process, recordings] : _processes_recordings) {
        for (const auto& recording : recordings) {
            if (std::chrono::duration_cast<std::chrono::nanoseconds>(recording.end_time - recording.start_time).count() < 1000) {
                continue;
            }

            res += "\t\t{ \"args\":{}, \"name\":\"";
            res += recording.name;

            if (recording.categories.size()) {
                res += "\", \"cat\":\"";
                for (std::ptrdiff_t i = 0; i < recording.categories.size() - 1; ++i) {
                    res += *(recording.categories.begin() + i);
                    res += ',';
                }
                res += *(recording.categories.end() - 1);
                res += "\", ";
            } else {
                res += "\", ";
            }
            res += "\"ph\":\"X\", ";
            res += "\"ts\":";

            res += std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(recording.start_time - _profiler_start_time).count() / 1000.0);
            res += ", ";
            res += "\"dur\":";
            res += std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(recording.end_time - recording.start_time).count() / 1000.0);
            res += ", ";
            res += "\"pid\": \"";
            res += process;
            res += "\", \"tid\":\"";
            if (!thread_names.contains(recording.thread_id)) {
                thread_names[recording.thread_id] = thread_names.size() + 1;
            }
            res += "Thread " + std::to_string(thread_names[recording.thread_id]);
            res += "\" },\n";
        }
    }
    if (_processes_recordings.size()) {
        res.pop_back(); // remove \n
        res.pop_back(); // remove ,
    }
    res += "\n\t]\n}";

    return res;
}
auto Profiler::save_as_trace_event_json(std::filesystem::path path) -> Utily::Result<void, Utily::Error> {
    if constexpr (Config::SKIP_PROFILE) {
        return {};
    }

    auto contents = Profiler::format_as_trace_event_json();
    std::ofstream fileStream(path, std::ios::out | std::ios::trunc);
    if (!fileStream) {
        return Utily::Error("Unable to open file for writing");
    }
    fileStream << contents;
    fileStream.close();

#if defined(CONFIG_TARGET_WEB)
    EM_ASM_({
        var filename = "app_trace.json";
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
#endif

    return {};
}

Profiler::~Profiler() {
    if constexpr (Config::SKIP_PROFILE) {
        return;
    }
    Profiler::save_as_trace_event_json(TRACE_FILE_NAME);
}

auto now = []() {
    thread_local std::chrono::steady_clock::time_point last_time = std::chrono::high_resolution_clock::now();
    auto curr = std::chrono::high_resolution_clock::now();
    if (curr <= last_time) {
        curr = last_time + std::chrono::nanoseconds(10);
    }
    last_time = curr;
    return curr;
};

Profiler::Timer::Timer(std::string function_name, std::vector<std::string_view> cats)
    : name(std::move(function_name))
    , start_time(now())
    , end_time(now())
    , thread_id(std::this_thread::get_id())
    , categories(cats) { }
Profiler::Timer::~Timer() {
    if constexpr (Config::SKIP_PROFILE) {
        return;
    }
    end_time = now();
    Profiler::instance().submit_timer(*this);
}