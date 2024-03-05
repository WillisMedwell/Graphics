#pragma once

#include <array>
#include <optional>
#include <span>

#include <Utily/Utily.hpp>
#include <glm/vec3.hpp>

#include "Config.hpp"
#include "Media/Sound.hpp"

namespace Core {

    class AudioManager
    {
    public:
        struct BufferHandle {
            int index = -1;
        };

        [[nodiscard]] auto init() -> Utily::Result<void, Utily::Error>;
        void stop();

        [[nodiscard]] auto load_sound_into_buffer(const Media::Sound& sound) -> Utily::Result<BufferHandle, Utily::Error>;
        [[nodiscard]] auto play_sound(BufferHandle buffer_handle, glm::vec3 pos = { 0, 0, 0 }) -> Utily::Result<void, Utily::Error>;

    private:
        constexpr static size_t MAX_BUFFERS = 1024;
        constexpr static size_t MAX_SOURCES = 256;

        bool _has_init = false;
        bool _has_stopped = false;

        std::optional<void*> _device = std::nullopt;
        std::optional<void*> _context = std::nullopt;

        struct Buffer {
            uint32_t id = std::numeric_limits<uint32_t>::max();
            bool is_populated = false;
            std::optional<std::chrono::milliseconds> duration = std::nullopt;
        };
        struct Source {
            uint32_t id = std::numeric_limits<uint32_t>::max();
            std::optional<BufferHandle> attached_buffer = std::nullopt;
            std::optional<std::chrono::steady_clock::time_point> expected_finish = std::nullopt;
        };

        std::array<Buffer, MAX_BUFFERS> _raw_buffers {};
        std::array<Source, MAX_SOURCES> _raw_sources {};

        std::span<Buffer> _buffers {};
        std::span<Source> _sources {};

        [[nodiscard]] auto init_device() -> Utily::Result<void, Utily::Error>;
        [[nodiscard]] auto init_context() -> Utily::Result<void, Utily::Error>;
        [[nodiscard]] auto init_buffers() -> Utily::Result<void, Utily::Error>;
        [[nodiscard]] auto init_sources() -> Utily::Result<void, Utily::Error>;

        void stop_device();
        void stop_context();
        void stop_buffers();
        void stop_sources();
    };
}