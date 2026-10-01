#include <portaudio.h>

#include <cmath>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "../libs/codec/include/audio_codec_factory.h"
#include "../libs/platform-ipc/include/ipc.h"
#include "git_info.h"

static volatile bool g_running = true;

static void handle_signal(int signal) { g_running = false; }

static bool checkPaError(PaError err, const char *message) {
  if (err == paNoError) {
    return true;
  }

  std::cerr << message << ":" << Pa_GetErrorText(err) << std::endl;
  return false;
}

int main() {
  std::cout << "Git Branch:  " << GIT_BRANCH << std::endl;
  std::cout << "Git Commit:  " << GIT_COMMIT << std::endl;

  std::signal(SIGINT, handle_signal);

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
  std::vector<uint8_t> encoded(encodedBytesPerFrame);

  std::cout << "Audio path source application" << std::endl;
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

  /* Open default input and output device. */
  // input channels - 1 microphone
  // ouput channels - 1 speaker
  // sample format  - signed 16-bit PCM

  err = Pa_OpenDefaultStream(&stream,
                             1,                     // input channels
                             1,                     // output channels
                             paInt16,               // sample format
                             codec->sampleRate(),   // sample rate
                             codec->frameSamples(), // frames per buffer
                             nullptr,               // no callback, blocking API
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

  std::cout << "Audio stream started, Press Ctrl+C to stop" << std::endl;

  uint64_t frameCount = 0;

  // Details required for IPC configuration.
  uint8_t myNodeId = 1;
  uint8_t dstNodeId = 2;
  uint16_t port = 7000;

  SDR::Ipc ipc;

  if (!ipc.create(myNodeId, port, "sdripc0-send")) {
    return 1;
  }

  bool ipc_send_ret;

  while (g_running) {
    err = Pa_ReadStream(stream, inputPcm.data(),
                        static_cast<unsigned long>(pcmSamplesPerFrame));

    if (err == paInputOverflow) {
      std::cerr << "Warning: input overflow, continuing" << std::endl;
      continue;
    }

    if (err != paNoError) {
      std::cerr << "Read error: " << Pa_GetErrorText(err) << std::endl;
      break;
    }

    // Encode PCM frame using codec.
    size_t writtenEncodedBytes = 0;

    bool ok =
        codec->encodeFrame(inputPcm.data(), inputPcm.size(), encoded.data(),
                           encoded.size(), writtenEncodedBytes);

    if (!ok) {
      std::cerr << "Codec encodeFrame failed" << std::endl;
      break;
    }

    ipc_send_ret = ipc.send(dstNodeId, encoded.data(), encoded.size());

    if (!ipc_send_ret) {
      std::cerr << "IPC send error: continuing" << std::endl;
    }
  }
}
