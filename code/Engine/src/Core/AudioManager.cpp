#include "Core/AudioManager.hpp"

#include "Profiler/Profiler.hpp"

namespace Core {

    auto AudioManager::init() -> Utily::Result<void, Utily::Error> {

        Profiler::Timer timer("Core::AudioManager::init()");

        if (_has_init) {
            return Utily::Error("Trying to reinitialise AudioManager.");
        }
        if (auto result = init_device(); result.has_error()) {
            return result.error();
        }
        if (auto result = init_context(); result.has_error()) {
            return result.error();
        }
        if (auto result = init_buffers(); result.has_error()) {
            return result.error();
        }
        if (auto result = init_sources(); result.has_error()) {
            return result.error();
        }
        _has_init = true;
        return {};
    }

    auto AudioManager::init_device() -> Utily::Result<void, Utily::Error> {
        Profiler::Timer timer("Core::AudioManager::init_device()");

        if (_device) {
            return Utily::Error("The sound device has already been initialised.");
        }
        _device = reinterpret_cast<void*>(alcOpenDevice(nullptr));
        if (*_device == nullptr) {
            _device = std::nullopt;
            return Utily::Error("Openal::alcOpenDevice() failed to open a sound device.");
        }
        return {};
    }
    auto AudioManager::init_context() -> Utily::Result<void, Utily::Error> {

        Profiler::Timer timer("Core::AudioManager::init_context()");

        if (_context) {
            return Utily::Error("The openal context has already been initialised.");
        } else if (!_device) {
            return Utily::Error("Could not init sound context. The sound device has not been initialised first.");
        }

        _context = reinterpret_cast<void*>(alcCreateContext(reinterpret_cast<ALCdevice*>(*_device), nullptr));
        if (*_context == nullptr) {
            _context = std::nullopt;
            return Utily::Error { "Could not init sound context. Openal::alcCreateContext() failed." };
        }
        alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(*_context));

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (auto error = alGetError(); error == ALC_INVALID_VALUE) {
                _context = std::nullopt;
                return Utily::Error { "Audio::Context::init() Openal::alcCreateContext() failed: cannot make additional context for this device." };
            } else if (error == ALC_INVALID_DEVICE) {
                _context = std::nullopt;
                return Utily::Error { "Audio::Context::init() failed. Openal::alcCreateContext() failed: invalid device." };
            } else if (error == ALC_INVALID_CONTEXT) {
                return Utily::Error { "Audio::Context::init() failed. Openal::alcMakeContextCurrent() failed: invalid context." };
            } else if (error != ALC_NO_ERROR) {
                return Utily::Error { "Audio::Context::init() failed. Openal::alcMakeContextCurrent() failed: no given reason." };
            }
        }
        return {};
    }
    auto AudioManager::init_buffers() -> Utily::Result<void, Utily::Error> {
        Profiler::Timer timer("Core::AudioManager::init_buffers()");

        if (_buffers.size()) {
            return Utily::Error("The sound buffers have already been initialised.");
        } else if (!_device || !_context) {
            return Utily::Error("The device or context has not been initialised.");
        }

        std::array<uint32_t, MAX_BUFFERS> buffer_ids { 0 };

        for (size_t num_buffers = buffer_ids.size(); num_buffers > 0; --num_buffers) {
            alGenBuffers(num_buffers, buffer_ids.data());
            if (glGetError() == AL_NO_ERROR && buffer_ids.front() != 0) {
                const size_t num_buffers = std::distance(buffer_ids.begin(), std::ranges::find(buffer_ids, 0));
                _buffers = std::span { _raw_buffers.begin(), _raw_buffers.begin() + num_buffers };

                std::transform(
                    buffer_ids.begin(),
                    buffer_ids.end(),
                    _buffers.begin(),
                    [](uint32_t id) { return Buffer(id); });
                break;
            }
        }
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_buffers.size() == 0) {
                return Utily::Error("Unable to create the audio buffers.");
            }
            bool all_buffers_are_valid = std::ranges::all_of(_buffers, [](Buffer& b) { return alIsBuffer(b.id); });
            if (!all_buffers_are_valid) {
                return Utily::Error("The buffers have been invalidated after they were created.");
            }
        }
        return {};
    }
    auto AudioManager::init_sources() -> Utily::Result<void, Utily::Error> {
        Profiler::Timer timer("Core::AudioManager::init_sources()");

        if (_sources.size()) {
            return Utily::Error("The sound sources have already been initialised.");
        } else if (!_device || !_context) {
            return Utily::Error("The device or context has not been initialised.");
        }

        std::array<uint32_t, MAX_SOURCES> source_ids { 0 };

        for (size_t num_sources = source_ids.size(); num_sources > 0; --num_sources) {
            alGenSources(num_sources, source_ids.data());
            if (glGetError() == AL_NO_ERROR && source_ids.front() != 0) {
                const size_t num_sources = std::distance(source_ids.begin(), std::ranges::find(source_ids, 0));
                _sources = std::span { _raw_sources.begin(), _raw_sources.begin() + num_sources };

                std::transform(
                    source_ids.begin(),
                    source_ids.end(),
                    _sources.begin(),
                    [](uint32_t id) { return Source(id); });

                break;
            }
        }
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_sources.size() == 0) {
                return Utily::Error("Unable to create the audio sources.");
            }
            bool all_sources_are_valid = std::ranges::all_of(_sources, [](const Source& s) { return alIsSource(s.id); });
            if (!all_sources_are_valid) {
                return Utily::Error("The sources have been invalidated after they were created.");
            }
        }
        return {};
    }

    void AudioManager::stop() {

        stop_sources();
        stop_buffers();
        stop_context();
        stop_device();
        _has_stopped = true;
    }

    void AudioManager::stop_device() {
        if (_device) {
            alcCloseDevice(reinterpret_cast<ALCdevice*>(*_device));
        }
        _device = std::nullopt;
    }
    void AudioManager::stop_context() {
        if (_context) {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(reinterpret_cast<ALCcontext*>(*_context));
        }
        _context = std::nullopt;
    }
    void AudioManager::stop_buffers() {
        if (_buffers.size()) {
            std::array<uint32_t, MAX_BUFFERS> buffer_ids;
            std::transform(_buffers.begin(), _buffers.end(), buffer_ids.begin(), [](Buffer& b) { return b.id; });

            alDeleteBuffers(_buffers.size(), buffer_ids.data());
            if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
                if (glGetError() != AL_NO_ERROR) {
                    throw std::runtime_error("failed to destroy sources and buffers");
                }
            }
        }
        _buffers = {};
    }
    void AudioManager::stop_sources() {
        if (_sources.size()) {
            std::array<uint32_t, MAX_SOURCES> source_ids;
            std::transform(_sources.begin(), _sources.end(), source_ids.begin(), [](Source& s) { return s.id; });

            alDeleteSources(_sources.size(), source_ids.data());
            if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
                if (glGetError() != AL_NO_ERROR) {
                    throw std::runtime_error("failed to destroy sources and buffers");
                }
            }
        }
        _sources = {};
    }

    auto AudioManager::load_sound_into_buffer(const Media::Sound& sound) -> Utily::Result<BufferHandle, Utily::Error> {
        Profiler::Timer timer("Core::AudioManager::load_sound_into_buffer()");

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_has_init) {
                return Utily::Error("Has not been initialsied");
            } else if (!_device || !_context || !_sources.size() || !_buffers.size()) {
                return Utily::Error("Has not successfully initialsied");
            }
        }

        for (int i = 0; i < _buffers.size(); ++i) {
            Buffer& buffer = _buffers[i];

            if (!buffer.is_populated) {
                alBufferData(buffer.id, (ALenum)sound.openal_format(), sound.raw_bytes().data(), sound.raw_bytes().size_bytes(), sound.frequency());

                if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
                    if (glGetError() != AL_NO_ERROR) {
                        return Utily::Error("Unable to load sound into buffer");
                    }
                }
                buffer.is_populated = true;

                if (sound.openal_format() == Media::Sound::FormatOpenal::stereo16) {
                    std::cout << "Stereo" << '\n';
                    buffer.duration = std::chrono::milliseconds { static_cast<int>(static_cast<float>(sound.raw_bytes().size_bytes()) / 4.0f / sound.frequency() * 1000.0f) };
                } else if (sound.openal_format() == Media::Sound::FormatOpenal::mono16) {
                    std::cout << "Mono" << '\n';
                    buffer.duration = std::chrono::milliseconds { static_cast<int>(static_cast<float>(sound.raw_bytes().size_bytes()) / 2.0f / sound.frequency() * 1000.0f) };
                } else {
                    return Utily::Error("Core::AudioManager::load_sound_into_buffer() failed. Unhandled format");
                }

                return BufferHandle { i };
            }
        }
        return Utily::Error("Unable to find empty buffer");
    }

    auto AudioManager::play_sound(BufferHandle buffer_handle, glm::vec3 pos, glm::vec3 vel) -> Utily::Result<Core::AudioManager::SourceHandle, Utily::Error> {
        Profiler::Timer timer("Core::AudioManager::play_sound()");

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_has_init) {
                return Utily::Error("Core::AudioManager::play_sound() failed. The manager has not been initialsied");
            } else if (!_device || !_context || !_sources.size() || !_buffers.size()) {
                return Utily::Error("Core::AudioManager::play_sound() failed. The manager tried, but has not successfully initialsied");
            }
        }

        const auto now = std::chrono::high_resolution_clock::now();

        auto is_source_playing = [&now](Source& source) {
            if (!source.expected_finish) {
                return false;
            } else if (now <= source.expected_finish.value()) {
                return true;
            }

            ALint state = 0;
            alGetSourcei(source.id, AL_SOURCE_STATE, &state);

            if (state != AL_PLAYING) {
                source.expected_finish = std::nullopt;
                return false;
            }
            return true;
        };

        for (int i = 0; i < _sources.size(); ++i) {
            Source& source = _sources[i];
            if (!is_source_playing(source)) {
                const Buffer& buffer = _buffers[buffer_handle.index];

                // attach buffer to source.
                if (!source.attached_buffer || source.attached_buffer.value().index != buffer_handle.index) {
                    alSourcei(source.id, AL_BUFFER, buffer.id);
                    source.attached_buffer = buffer_handle;
                }
                alSource3f(source.id, AL_POSITION, pos.x, pos.y, pos.z);
                alSource3f(source.id, AL_VELOCITY, vel.x, vel.y, vel.z);
                {
                    Profiler::Timer player("alSourcePlay()");
                    alSourcePlay(source.id);
                }

                source.expected_finish = std::chrono::steady_clock::now()
                    + buffer.duration.value_or(std::chrono::milliseconds(0));

                if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
                    if (glGetError() != AL_NO_ERROR) {
                        return Utily::Error("Core::AudioManager::play_sound() failed. Unable to play sound.");
                    }
                }

                return Core::AudioManager::SourceHandle { i };
            }
        }
        return Utily::Error("Core::AudioManager::play_sound() failed. No free sources avaliable");
    }

    void AudioManager::set_listener_properties(const ListenerProperties& listener_properties) {
        Profiler::Timer timer("Core::AudioManager::set_listener_properties()");

        constexpr static auto default_val = glm::vec3(std::numeric_limits<float>::min());
        static ListenerProperties last = { default_val, default_val, default_val };

        if (listener_properties.pos && last.pos.value() != listener_properties.pos.value()) {
            alListener3f(AL_POSITION, listener_properties.pos.value().x, listener_properties.pos.value().y, listener_properties.pos.value().z);
            last.pos = listener_properties.pos;
        }
        if (listener_properties.vel && last.vel.value() != listener_properties.vel.value()) {
            alListener3f(AL_VELOCITY, listener_properties.vel.value().x, listener_properties.vel.value().y, listener_properties.vel.value().z);
            last.vel = listener_properties.vel;
        }
        if (listener_properties.dir && last.dir.value() != listener_properties.dir.value()) {
            glm::vec3 orientation[2] = { listener_properties.dir.value(), listener_properties.UP_DIR };
            alListenerfv(AL_ORIENTATION, reinterpret_cast<float*>(orientation));
            last.dir = listener_properties.dir;
        }
    }

    auto AudioManager::set_source_motion(SourceHandle source_handle, glm::vec3 pos, glm::vec3 vel) -> Utily::Result<void, Utily::Error> {
        Profiler::Timer timer("Core::AudioManager::set_source_motion()");

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            Profiler::Timer timer2("alIsSource() *debug*");
            if (source_handle.index >= _sources.size()) {
                return Utily::Error("Core::AudioManager::set_source_motion() failed. The source is out of range.");
            } else if (!alIsSource(this->_sources[source_handle.index].id)) {
                return Utily::Error("Core::AudioManager::set_source_motion() failed. OpenAL has invalidated the source.");
            }
        }

        auto& source = _sources[source_handle.index];
        const uint32_t& source_id = source.id;

        if (source.pos != pos) {
            alSource3f(source_id, AL_POSITION, pos.x, pos.y, pos.z);
            source.pos = pos;
        }

        if (source.vel != vel) {
            alSource3f(source_id, AL_VELOCITY, vel.x, vel.y, vel.z);
            source.vel = vel;
        }

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (glGetError() != AL_NO_ERROR) {
                return Utily::Error("Core::AudioManager::set_source_motion() failed. Unable to play sound.");
            }
        }
        return {};
    }
}