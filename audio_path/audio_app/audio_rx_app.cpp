#include <cstddef>
#include <include/audio_codec_factory.h>
#include <portaudio.h>

#include <array>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "git_info.h"

static volatile bool g_running = true;

static void handleSignal(int) { g_running = false; }

static bool checkPaError(PaError err, const char *message) {
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

  std::vector<int16_t> inputPcm(pcmSamplesPerFrame);
  std::vector<int16_t> outputPcm(pcmSamplesPerFrame);
  std::vector<int8_t> encoded(encodedBytesPerFrame);

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

  PaStream *stream = nullptr;

  err = Pa_OpenDefaultStream(&stream, 0, 0, paInt16, codec->sampleRate(),
                             codec->frameSample(), nullptr, nullptr)

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
}