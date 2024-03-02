#include "Media/Sound.hpp"

#include <Utily/Utily.hpp>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

namespace Media {
    auto Sound::init_from_wav(const std::vector<uint8_t>& encoded_wav) -> Utily::Result<void, Utily::Error> {

        uint32_t num_channels = 0;
        uint32_t sample_rate = 0;
        uint64_t frame_count = 0;

        int16_t* dr_wav_ptr = drwav_open_memory_and_read_pcm_frames_s16(encoded_wav.data(), encoded_wav.size(), &num_channels, &sample_rate, &frame_count, nullptr);

        auto dr_wav_data_i16 = std::span<const int16_t> { dr_wav_ptr, static_cast<size_t>(num_channels * frame_count) };

        if (!dr_wav_ptr) {
            return Utily::Error("Media::Sound::init_from_wave() failed. Dr wav failed to parse the file.");
        }

        _data.resize(dr_wav_data_i16.size());
        std::ranges::copy(dr_wav_data_i16, _data.begin());
        drwav_free(dr_wav_ptr, nullptr);

        _frequency = sample_rate;

        if (num_channels == 1) {
            _format = Format::mono_i16;
        } else if (num_channels == 2) {
            _format = Format::stereo_i16;
        } else {
            _data.clear();
            _frequency = 0;
            return Utily::Error("Media::Sound::init_from_wave() failed. Too many channels in .wav data. Can only handle 2 channels.");    
        }
        return {};
    }

}