#include "Audio/Device.hpp"

#include "Config.hpp"

namespace Audio {

    void Device::debug_validate() {
        if constexpr (Config::DEBUG_LEVEL != Config::DebugInfo::none) {
            if (!_device.value_or(nullptr)) {
                throw std::runtime_error("Audio::Device not initialised");
            }
        }
    }

    auto Device::init() -> Utily::Result<void, Utily::Error> {
        if (_device) {
            stop();
        }
        _device = reinterpret_cast<void*>(alcOpenDevice(nullptr));
        if (!_device.value_or(nullptr)) {
            _device = std::nullopt;
            return Utily::Error { "Device::init() failed. Openal::alcOpenDevice() failed to open a sound device." };
        }
        return {};
    }

    void Device::stop() {
        if (_device) {
            alcCloseDevice(reinterpret_cast<ALCdevice*>(*_device));
        }
        _device = std::nullopt;
    }

    Device::~Device() {
        stop();
    }
}