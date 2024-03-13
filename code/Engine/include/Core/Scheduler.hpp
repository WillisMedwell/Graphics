#include <algorithm>
#include <array>
#include <bit>
#include <functional>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <thread>
#include <variant>
#include <vector>

#include "Config.hpp"

namespace Core {

    class Scheduler
    {
    public:
        using SimpleFunctionPtr = void (*)();
        using Task = std::variant<SimpleFunctionPtr, std::function<void()>>;

        static auto create(size_t num_threads = std::thread::hardware_concurrency())
            -> std::optional<Scheduler> {
            if (OptAtomicSize temp = 1; !temp.value().is_lock_free()) {
                throw std::runtime_error(
                    "This has lock, as such it cannot be moved. Therefore the move "
                    "semantics workaround is invalidated.");
            }
            if constexpr (Config::PLATFORM == Config::TargetPlatform::web) {
                num_threads = 0;
            }

            return Scheduler(M {
                .tasks = {},
                .threads = {},
                .num_threads = num_threads,
                .current_task_buffer = null_opt_atomic(),
            });
        }

        void add_task(SimpleFunctionPtr task) {
            if (get_current_task().has_value()) {
                throw std::runtime_error("trying to add task when already lauched");
            }
            _m.tasks.emplace_back(task);
        }

        void add_task(Task task) {
            if (get_current_task().has_value()) {
                throw std::runtime_error("trying to add task when already lauched");
            }
            _m.tasks.emplace_back(task);
        }

        void launch_threads() {
            if (get_current_task().has_value()) {
                return;
            }

            get_current_task().emplace(0);

            auto execute = [this]() {
                for (;;) {
                    auto task_id =
                        get_current_task()->fetch_add(1, std::memory_order_seq_cst);

                    bool has_tasks_remaining = task_id < _m.tasks.size();

                    if (has_tasks_remaining) {
                        std::string timer_name = "Scheduler::Task(" + std::to_string(task_id) + ")";
                        Profiler::Timer timer(timer_name);
                        auto& task = _m.tasks.at(task_id);
                        std::visit([](auto& task) { task(); }, task);
                    } else {
                        return;
                    }
                }
            };
            auto start_thread = [&]() { return std::thread(execute); };

            // start the other threads
            std::generate_n(std::back_inserter(_m.threads), _m.num_threads, start_thread);
        }

        void wait_for_threads() {
            // get the main thread going too.
            [this]() {
                for (;;) {
                    auto task_id =
                        get_current_task()->fetch_add(1, std::memory_order_seq_cst);

                    bool has_tasks_remaining = task_id < _m.tasks.size();

                    if (has_tasks_remaining) {
                        auto& task = _m.tasks.at(task_id);
                        std::visit([](auto& task) { task(); }, task);
                    } else {
                        return;
                    }
                }
            }();

            std::ranges::for_each(_m.threads, &std::thread::join);

            _m.threads.resize(0);
            _m.tasks.resize(0);
            get_current_task() = std::nullopt;
        }

        Scheduler(Scheduler&&) = default;

    private:
        using OptAtomicSize = std::optional<std::atomic<size_t>>;
        using OptAtomicSizeBuffer = std::array<std::byte, sizeof(OptAtomicSize)>;

        constexpr static auto null_opt_atomic = []() {
            OptAtomicSize empty = std::nullopt;
            OptAtomicSizeBuffer buffer =
                *reinterpret_cast<OptAtomicSizeBuffer*>(&empty);
            return buffer;
        };

        struct M {
            std::vector<Task> tasks;
            std::vector<std::thread> threads;
            size_t num_threads;
            alignas(OptAtomicSize) OptAtomicSizeBuffer current_task_buffer;
        } _m;

        auto get_current_task() -> OptAtomicSize& {
            return *reinterpret_cast<OptAtomicSize*>(_m.current_task_buffer.data());
        }

        explicit Scheduler(M&& m)
            : _m(std::move(m)) { }

        Scheduler(const Scheduler&) = delete;
    };
}
