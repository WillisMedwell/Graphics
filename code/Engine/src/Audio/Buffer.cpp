#include "Audio/Buffer.hpp"

#include "Config.hpp"

namespace Audio {
    auto Buffer::init() noexcept -> Utily::Result<void, Utily::Error> {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id) {
                return Utily::Error("Audio::Buffer::init() failed. Already initialised, do not re-init without calling stop() first.");
            }
        }
        stop();
        _id = INVALID_ID;
        alGenBuffers(1, &_id.value());
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (auto error = alGetError(); error == AL_INVALID_VALUE) {
                _id = std::nullopt;
                return Utily::Error("Audio::Buffer::init() failed. alGenBuffers() failed: Buffer array isn't large enough.");
            } else if (error == AL_OUT_OF_MEMORY) {
                _id = std::nullopt;
                return Utily::Error("Audio::Buffer::init() failed. alGenBuffers() failed: Ran out of memory.");
            } else if (_id.value() == INVALID_ID) {
                _id = std::nullopt;
                return Utily::Error("Audio::Buffer::init() failed. alGenBuffers() failed: no error message from openal.");
            }
        }
        return {};
    }

    void Buffer::stop() noexcept {
        if (_id) {
            alDeleteBuffers(1, &_id.value());
        }
        _id = std::nullopt;
    }

    auto Buffer::load_sound(const Media::Sound& sound) -> Utily::Result<void, Utily::Error> {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ID) == INVALID_ID) {
                return Utily::Error("Audio::Buffer::load_sound() failed. The Audio::Buffer is not valid.");
            }
        }
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (sound.size_in_bytes() == 0) {
                return Utily::Error("Audio::Buffer::load_sound() failed. The Sound passed in has no data.");
            }
        }
        alBufferData(*_id, (ALenum)sound.openal_format(), sound.data(), sound.size_in_bytes(), sound.frequency());
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (auto error = glGetError(); error == AL_OUT_OF_MEMORY) {
                return Utily::Error("Audio::Buffer::load_sound() failed. alBufferData() failed: Ran out of memory.");
            } else if (error == AL_INVALID_ENUM) {
                return Utily::Error("Audio::Buffer::load_sound() failed. alBufferData() failed: The sound format does not exist.");
            } else if (error == AL_INVALID_VALUE) {
                if (sound.data() == nullptr) {
                    return Utily::Error("Audio::Buffer::load_sound() failed. alBufferData() failed: The Sound.data() is nullptr.");
                } else {
                    return Utily::Error("Audio::Buffer::load_sound() failed. alBufferData() failed: The Sound::size_of_bytes() is invaid for the format.");
                }
            }
        }
        return {};
    }

    Buffer::~Buffer() {
        stop();
    }
}