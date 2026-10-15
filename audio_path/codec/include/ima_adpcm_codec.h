#pragma once

#include "codec_interface.h"

namespace SDR {

class ImaAdpcmCodec final : public AudioCodec {
 public:
  AudioCodecId id() const override;
  const char* name() const override;

  uint32_t sampleRate() const override;
  size_t frameSamples() const override;
  uint8_t channels() const override;

  size_t encodedBytesForFrame() const override;

  bool encodeFrame(const int16_t* pcm, size_t pcmSamples, uint8_t* encoded,
                   size_t encodedCapacity, size_t& writtenBytes) override;

  bool decodeFrame(const uint8_t* encoded, size_t encodedBytes, int16_t* pcm,
                   size_t pcmCapacitySamples, size_t& writtenSamples) override;

 private:
  explicit ImaAdpcmCodec(AudioCodecConfig config = {});

  friend class AudioCodecFactory;

  AudioCodecConfig config_;
};

}  // namespace SDR