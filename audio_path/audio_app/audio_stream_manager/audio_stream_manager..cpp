#include "audio_stream_manager.h"

#include <portaudio.h>

#include <iostream>

namespace SDR {

AudioStreamManager& AudioStreamManager::getInstance() {
    static AudioStreamManager instance;
    return instance;
}

bool AudioStreamManager::checkPaError(PaError err, const char* message) {
    if (err == paNoError) {
        return true;
    }

    std::cerr << message << ":" << Pa_GetErrorText(err) << std::endl;
    return false;
}

AudioStreamStatus AudioStreamManager::Init(const AudioStreamConfig& config) {
    audio_stream_config = config;

    if (AudioStreamManager::is_initialized) {
        std::cerr << "Audio Stream Manager already has been initialized."
                  << std::endl;
        return AudioStreamStatus::Error;
    }

    PaError err;
    err = Pa_Initialize();

    if (!checkPaError(err, "Pa_Initialize failed.")) {
        return AudioStreamStatus::Error;
    }

    err = Pa_OpenDefaultStream(
        &audio_stream, config.input_channels, config.output_channels, paInt16,
        config.sample_rate, config.frames_per_buffer, nullptr, nullptr);

    if (!checkPaError(err, "Pa_OpenDefaultStream failed.")) {
        Pa_Terminate();
        return AudioStreamStatus::Error;
    }

    err = Pa_StartStream(audio_stream);

    if (!checkPaError(err, "Pa_StartStream failed.")) {
        Pa_CloseStream(audio_stream);
        Pa_Terminate();
        return AudioStreamStatus::Error;
    }
    this->is_initialized = true;

    return AudioStreamStatus::NoError;
}

AudioStreamStatus AudioStreamManager::readAudioFrames(
    std::vector<int16_t>& pcmBuffer) {
    if (!AudioStreamManager::is_initialized) {
        std::cerr << "Audio Stream Manager has not been initialized."
                  << std::endl;
        return AudioStreamStatus::Error;
    }

    PaError err;
    err = Pa_ReadStream(audio_stream, pcmBuffer.data(),
                        audio_stream_config.frames_per_buffer);

    if (!checkPaError(err, "Pa_ReadStream. ")) {
        return AudioStreamStatus::Error;
    }

    return AudioStreamStatus::NoError;
}

AudioStreamStatus AudioStreamManager::writeAudioFrames(
    std::vector<int16_t>& pcmBuffer) {
    if (!AudioStreamManager::is_initialized) {
        std::cerr << "Audio Stream Manager has not been initialized."
                  << std::endl;
        return AudioStreamStatus::Error;
    }

    PaError err;
    err = Pa_WriteStream(audio_stream, pcmBuffer.data(),
                         audio_stream_config.frames_per_buffer);

    if (!checkPaError(err, "Pa_WriteStrea.")) {
        return AudioStreamStatus::Error;
    }

    return AudioStreamStatus::NoError;
}

// private constructor.
AudioStreamManager::AudioStreamManager() {
    this->is_initialized = false;
}

// private destructor.
AudioStreamManager::~AudioStreamManager() {
    if (is_initialized) {
        Pa_StopStream(audio_stream);
        Pa_CloseStream(audio_stream);
        Pa_Terminate();
    }
}

}  // namespace SDR
