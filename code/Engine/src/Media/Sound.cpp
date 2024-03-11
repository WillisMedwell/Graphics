#include "Media/Sound.hpp"

#include "Profiler/Profiler.hpp"
#include <Utily/Utily.hpp>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

namespace Media {
    auto Sound::create(std::filesystem::path wav_path) noexcept -> Utily::Result<Sound, Utily::Error> {
        Profiler::Timer timer("Media::Sound::create()");
        // 1. Load the raw contents of the wav file.
        // 2. Decode the contents use dr-wav.
        // 3. Compress stereo audio into mono audio and copy into buffer.
        // 4. Free resources
        // 5. Create Media::Sound

        // 1.
        auto load_file_result = Utily::FileReader::load_entire_file(wav_path);
        if (load_file_result.has_error()) {
            return load_file_result.error();
        }
        const auto& encoded_wav = load_file_result.value();

        // 2.
        uint32_t num_channels = 0;
        uint32_t sample_rate = 0;
        uint64_t frame_count = 0;

        int16_t* dr_wav_ptr = drwav_open_memory_and_read_pcm_frames_s16(encoded_wav.data(), encoded_wav.size(), &num_channels, &sample_rate, &frame_count, nullptr);
        if (!dr_wav_ptr) {
            return Utily::Error("Media::Sound::init_from_wave() failed. Dr wav failed to parse the file.");
        }
        auto dr_wav_data_i16 = std::span<const int16_t> { dr_wav_ptr, static_cast<size_t>(num_channels * frame_count) };

        // 3.
        std::vector<int16_t> data;
        if (num_channels == 2) {
            data.resize(dr_wav_data_i16.size() / 2);
            auto iter = data.begin();
            // Combine to mono by taking average of the two channels.
            for (int i = 0; i < dr_wav_data_i16.size(); i += 2) {
                *iter = (dr_wav_data_i16[i] / 2) + (dr_wav_data_i16[i + 1] / 2);
                ++iter;
            }
            num_channels = 1;
        } else {
            data.resize(dr_wav_data_i16.size());
            std::ranges::copy(dr_wav_data_i16, data.begin());
        }

        // 4.
        drwav_free(dr_wav_ptr, nullptr);

        // 5.
        return Sound(M {
            .data = std::move(data),
            .frequency = sample_rate,
            .format = Format::mono_i16,
        });
    }
}