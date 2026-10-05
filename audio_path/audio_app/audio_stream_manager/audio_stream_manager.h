#include <portaudio.h>

#include <cstdint>
#include <iostream>
#include <vector>

namespace SDR {

enum class AudioStreamStatus : int {
    NoError = 0,
    Error = -1,
    // TODO (Kavindu): fill required error codes.
};

struct AudioStreamConfig {
    uint8_t input_channels = 1;
    uint8_t output_channels = 1;
    double sample_rate = 16000;
    unsigned long frames_per_buffer = 320;
};

class AudioStreamManager {
public:
    static AudioStreamManager& getInstance();

    // Singleton pattern, delete copy and move constructors.
    AudioStreamManager(const AudioStreamManager&) = delete;
    AudioStreamManager& operator=(const AudioStreamManager&) = delete;
    AudioStreamManager(AudioStreamManager&&) = delete;
    AudioStreamManager& operator=(AudioStreamManager&&) = delete;

    AudioStreamStatus Init(const AudioStreamConfig& config);

    AudioStreamStatus readAudioFrames(std::vector<int16_t>& pcmBuffer);
    AudioStreamStatus writeAudioFrames(std::vector<int16_t>& pcmBuffer);

    // AudioStreamStatus checkError(AudioStreamError err, const char* message);

private:
    AudioStreamConfig audio_stream_config;
    PaStream* audio_stream = nullptr;

    bool is_initialized;

    AudioStreamManager();
    ~AudioStreamManager();

    bool checkPaError(PaError err, const char* message);
};

}  // namespace SDR
