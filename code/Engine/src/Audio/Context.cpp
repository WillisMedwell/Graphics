#include "Audio/Context.hpp"

#include "Config.hpp"

namespace Audio {

    void Context::debug_validate() {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_context.value_or(nullptr)) {
                throw std::runtime_error("Audio::Context not initialised");
            }
        }
    }

    auto Context::init(Audio::Device& device) -> Utily::Result<void, Utily::Error> {
        if (_context) {
            stop();
        }
        _context = reinterpret_cast<void*>(alcCreateContext(reinterpret_cast<ALCdevice*>(device.unsafe_device_handle()), nullptr));
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_context.value_or(nullptr)) {
                _context = std::nullopt;
                return Utily::Error { "Audio::Context::init() failed. Openal::alcCreateContext() failed: no error message from openal." };
            }
        }
        alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(*_context));

        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            /*
            Need to set current context before glGetError() can be called,
            hence why we do createContext() fail checks after MakeContextCurrent().
            */
            if (auto error = alGetError(); error == ALC_INVALID_VALUE) {
                _context = std::nullopt;
                return Utily::Error { "Audio::Context::init() failed. Openal::alcCreateContext() failed: cannot make additional context for this device." };
            } else if (error == ALC_INVALID_DEVICE) {
                _context = std::nullopt;
                return Utily::Error { "Audio::Context::init() failed. Openal::alcCreateContext() failed: invalid device." };
            } else if (error == ALC_INVALID_CONTEXT) {
                return Utily::Error { "Audio::Context::init() failed. Openal::alcMakeContextCurrent() failed: invalid context." };
            }
        }
        return {};
    }

    void Context::stop() {
        if (_context) {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(reinterpret_cast<ALCcontext*>(*_context));
        }
        _context = std::nullopt;
    }
    Context::~Context() {
        stop();
    }
}