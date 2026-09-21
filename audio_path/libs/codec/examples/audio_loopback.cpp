#include <include/ima_adpcm_codec.h>
#include <portaudio.h>

#include <array>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "git_info.h"

static bool g_running = true;

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

  std::unique_ptr<SDR::AudioCodec> codec =
      std::make_unique<SDR::ImaAdpcmCodec>(config);

  const size_t pcmSamplesPerFrame = codec->frameSamples();
  const size_t encodedBytesPerFrame = codec->encodedBytesForFrame();

  std::vector<int16_t> inputPcm(pcmSamplesPerFrame);
  std::vector<int16_t> outputPcm(pcmSamplesPerFrame);
  std::vector<uint8_t> encoded(encodedBytesPerFrame);

  std::cout << "Codec loopback application" << std::endl;
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

  /* Open default input and output device. */
  // input channels - 1 microphone
  // ouput channels - 1 speaker
  // sample format  - signed 16-bit PCM

  err = Pa_OpenDefaultStream(&stream,
                             1,                      // input channels
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

  std::cout << "Realtime audio loopback started." << std::endl;
  std::cout << "Mic -> encode -> decode -> speaker" << std::endl;
  std::cout << "Press Ctrl+C to stop." << std::endl;

  uint64_t frameCount = 0;

  while (g_running) {
    // Read one frame from the microphone.
    err = Pa_ReadStream(stream, inputPcm.data(),
                        static_cast<unsigned long>(pcmSamplesPerFrame));

    if (err == paInputOverflowed) {
      std::cerr << "Warning: input overflow, continuing..." << std::endl;
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

    // Decode encoded frame back to PCM.
    size_t writtenPcmSamples = 0;

    ok = codec->decodeFrame(encoded.data(), writtenEncodedBytes,
                            outputPcm.data(), outputPcm.size(),
                            writtenPcmSamples);

    if (!ok) {
      std::cerr << "Codec decodeFrame failed" << std::endl;
      break;
    }

    // Play decoded PCM frame.
    err = Pa_WriteStream(stream, outputPcm.data(),
                         static_cast<unsigned long>(writtenPcmSamples));

    if (err == paOutputUnderflowed) {
      std::cerr << "Warning: output underflow, continuing..." << std::endl;
      continue;
    }

    if (err != paNoError) {
      std::cerr << "Write error: " << Pa_GetErrorText(err) << std::endl;
      break;
    }

    frameCount++;

    if ((frameCount % 50) == 0) {
      std::cout << "Frames processed: " << frameCount
                << " | PCM: " << pcmSamplesPerFrame * sizeof(int16_t)
                << " bytes"
                << " | encoded: " << writtenEncodedBytes << " bytes"
                << std::endl;
    }
  }

  Pa_StopStream(stream);
  Pa_CloseStream(stream);
  Pa_Terminate();

  std::cout << "Audio loopback stopped." << std::endl;

  return 0;
}
