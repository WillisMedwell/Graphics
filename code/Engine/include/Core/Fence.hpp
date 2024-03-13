#pragma once

#include "../Config.hpp"
#include "Profiler/Profiler.hpp"

#include <optional>
#include <utility>

namespace Core {
    class Fence
    {
    public:
        Fence() = default;
        Fence(const Fence&) = delete;
        Fence(Fence&& other) noexcept
            : _id(std::exchange(other._id, std::nullopt)) {
        }

        void init() {
            _id = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }

        bool has_finished() {
            if (!_id) {
                return true;
            }

            auto wait_status = glClientWaitSync(*_id, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
            if (wait_status == GL_ALREADY_SIGNALED || wait_status == GL_CONDITION_SATISFIED) {
                glDeleteSync(*_id);
                _id = std::nullopt;
                return true;
            }
            return false;
        }
        void wait_for_sync() {
            Profiler::Timer("Core::Fence::wait_for_sync()");
            if (_id) {
                glWaitSync(*_id, 0, GL_TIMEOUT_IGNORED);
                glDeleteSync(*_id);
                _id = std::nullopt;
            }
        }
        ~Fence() {
            if (_id) {
                glDeleteSync(*_id);
            }
        }

    private:
        std::optional<GLsync> _id;
    };
}