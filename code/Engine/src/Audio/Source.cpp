#include "Audio/Source.hpp"

#include "Config.hpp"

namespace Audio {

    auto Source::init() -> Utily::Result<void, Utily::Error> {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ID) != INVALID_ID) {
                return Utily::Error("Audio::Source::init() failed. Trying to reinit.");
            }
        }
        stop();

        _id = INVALID_ID;
        alGenSources(1, &_id.value());
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (auto error = alGetError(); error == AL_OUT_OF_MEMORY) {
                return Utily::Error("Audio::Source::init() failed. alGenSources() failed: Openal ran out of memory.");
            } else if (error == AL_INVALID_VALUE) {
                return Utily::Error("Audio::Source::init() failed. alGenSources() failed: Not enough memory resources or bad id ptr.");
            } else if (error == AL_INVALID_OPERATION) {
                return Utily::Error("Audio::Source::init() failed. alGenSources() failed: There is no current context.");
            } else if (_id.value_or(INVALID_ID) == INVALID_ID) {
                return Utily::Error("Audio::Source::init() failed. Openal failed to set the id.");
            }
        }
        return {};
    }
    auto Source::bind(Audio::Buffer& buffer) -> Utily::Result<void, Utily::Error> {

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ID) == INVALID_ID) {
                return Utily::Error("Audio::Source::bind() failed. The source has not be initialised.");
            } else if (buffer._id.value_or(Audio::Buffer::INVALID_ID) == Audio::Buffer::INVALID_ID) {
                return Utily::Error("Audio::Source::bind() failed. The buffer passed in has not be initialised.");
            }
        }
        alSourcei(_id.value(), AL_BUFFER, buffer._id.value());
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (auto error = glGetError(); error == AL_INVALID_VALUE) {
                return Utily::Error("Audio::Source::bind() failed. Openal value is out of range.");
            } else if (error == AL_INVALID_ENUM) {
                return Utily::Error("Audio::Source::bind() failed. Openal says bad enum.");
            } else if (error == AL_INVALID_NAME) {
                return Utily::Error("Audio::Source::bind() failed. Openal does not recognise the source id.");
            } else if (error == AL_INVALID_OPERATION) {
                return Utily::Error("Audio::Source::bind() failed. Openal has no context.");
            }
        }
        _has_bound_buffer = true;
        return {};
    }

    void Source::unbind() {
        if (_id) {
            alSourcei(_id.value(), AL_BUFFER, 0);
        }
        _has_bound_buffer = false;
    }
    auto Source::play() -> Utily::Result<void, Utily::Error> {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (_id.value_or(INVALID_ID) == INVALID_ID) {
                return Utily::Error("Audio::Source::play() failed. The source has not been initialised.");
            } else if (!_has_bound_buffer) {
                return Utily::Error("Audio::Source::play() failed. There is no buffer currently bound to this source.");
            }
        }
        alSourcePlay(_id.value());

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (auto error = alGetError(); error == AL_INVALID_NAME) {
                return Utily::Error("Audio::Source::play() failed. The id has been invalidated -> e.g. someone has called alDeleteSource() for this id.");
            } else if (error == AL_INVALID_OPERATION) {
                return Utily::Error("Audio::Source::play() failed. There is no current context.");
            }
        }
        return {};
    }
    void Source::stop() {
        if (_id) {
            alSourceStop(_id.value());
            alDeleteSources(1, &_id.value());
        }
        _id = std::nullopt;
        _has_bound_buffer = false;
    }
    Source::~Source() {
        stop();
    }

}
