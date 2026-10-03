#include <libs/codec/include/audio_codec_factory.h>
#include <libs/platform-ipc/include/ipc.h>
#include <portaudio.h>

#include <array>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "git_info.h"

static volatile bool g_running = true;

static void handleSignal(int) {
    g_running = false;
}

static bool checkPaError(PaError err, const char* message) {
    if (err == paNoError) {
        return true;
    }

    std::cerr << message << ": " << Pa_GetErrorText(err) << std::endl;
    return false;
}

int main() {
    // Project version details.
    std::cout << "Git Branch   : " << PROJECT_GIT_BRANCH << std::endl;
    std::cout << "Git Commit Id: " << PROJECT_GIT_COMMIT << std::endl;

    std::signal(SIGINT, handleSignal);

    /*
     *  ------ Audio configuration ------
     * 16000 Hz sample rate
     * 320 samples per frame
     * 20ms per frame
     * mono audio
     * ----------------------------------
     */

    SDR::AudioCodecConfig config;
    config.sampleRate = 16000;
    config.frameSamples = 320;
    config.channels = 1;

    auto codec_id = SDR::AudioCodecId::IMA_ADPCM;

    auto codec = SDR::AudioCodecFactory::create(codec_id, config);

    if (!codec) {
        // unsupported codec
        return 1;
    }

    const size_t pcmSamplesPerFrame = codec->frameSamples();
    const size_t encodedBytesPerFrame = codec->encodedBytesForFrame();

    // std::vector<int16_t> inputPcm(pcmSamplesPerFrame);
    std::vector<int16_t> outputPcm(pcmSamplesPerFrame);
    std::vector<uint8_t> encoded(encodedBytesPerFrame);

    std::cout << "Audio path sink application" << std::endl;
    std::cout << "Codec: " << codec->name() << std::endl;
    std::cout << "Sample rate: " << codec->sampleRate() << " Hz" << std::endl;
    std::cout << "Frame samples: " << codec->frameSamples() << std::endl;
    std::cout << "PCM frame bytes: " << codec->frameSamples() * sizeof(int16_t)
              << std::endl;
    std::cout << "Encoded frame bytes: " << codec->encodedBytesForFrame()
              << std::endl;

    PaError err = Pa_Initialize();

    if (!checkPaError(err, "Pa_Initialize failed")) {
        return 1;
    }

    PaStream* stream = nullptr;

    err = Pa_OpenDefaultStream(&stream,
                               0,                      // input channels
                               1,                      // output channels
                               paInt16,                // sample format
                               codec->sampleRate(),    // sample rate
                               codec->frameSamples(),  // frames per buffer
                               nullptr,  // no callback, blocking API
                               nullptr);
    if (!checkPaError(err, "Pa_OpenDefaultStream failed")) {
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);

    if (!checkPaError(err, "Pa_StartStream failed")) {
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    uint64_t frameCount = 0;

    // Details required for IPC configuration.
    uint8_t myNodeId = 2;
    uint8_t dstNodeId = 1;
    uint16_t port = 7000;

    SDR::Ipc ipc;

    if (!ipc.create(myNodeId, port, "sdripc0-recv")) {
        return 1;
    }

    std::cout << "Realtime audio rx app started." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    // std::array<uint8_t, 2048> buffer{};

    size_t received = 0;
    bool decode_ret = false;
    size_t writtenPcmSamples = 0;

    while (g_running) {
        received = ipc.receive(encoded.data(), encoded.size());

        if (received != encoded.size()) {
            std::cerr << "Receive failed. Only received " << received << "/"
                      << encoded.size() << "bytes.\n";
            continue;
        }

        decode_ret =
            codec->decodeFrame(encoded.data(), received, outputPcm.data(),
                               outputPcm.size(), writtenPcmSamples);

        if (!decode_ret) {
            std::cerr << "Codec decodeFrame failed." << std::endl;
            continue;
        }

        // Play decoded PCM frame.
        err = Pa_WriteStream(stream, outputPcm.data(),
                             static_cast<unsigned long>(writtenPcmSamples));

        if (err == paOutputUnderflowed) {
            std::cerr << "Warning: output underflow, continuing..."
                      << std::endl;
            continue;
        }

        if (err != paNoError) {
            std::cerr << "Write error:" << Pa_GetErrorText(err) << std::endl;
            break;
        }

        frameCount++;

        if ((frameCount % 50) == 0) {
            std::cout << "Frames processed: " << frameCount
                      << " | PCM: " << pcmSamplesPerFrame * sizeof(int16_t)
                      << " bytes"
                      << " | encoded: " << received << " bytes" << std::endl;
        }
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "Audio rx app stopped." << std::endl;

    return 0;
}
