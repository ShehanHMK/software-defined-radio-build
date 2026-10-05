#pragma once

#include <cstddef>
#include <cstdint>

namespace SDR {

enum class AudioCodecId : uint8_t {
    PCM16 = 0,
    IMA_ADPCM = 1,
};


struct AudioCodecConfig {           // Default values.
    uint32_t sampleRate = 16000;
    size_t frameSamples = 320;      // 20ms at 16khz.
    uint8_t channels = 1;           // mono for now.
};


class AudioCodec {
public:
    virtual ~AudioCodec() = default;

    virtual AudioCodecId id() const = 0;
    virtual const char *name() const = 0;


    virtual uint32_t sampleRate() const = 0;
    virtual size_t frameSamples() const  = 0;
    virtual uint8_t channels() const = 0;

    virtual size_t encodedBytesForFrame() const = 0;


    virtual bool encodeFrame(
        const int16_t *pcm,
        size_t pcmSamples,
        uint8_t *encoded,
        size_t encodedCapacity,
        size_t &writtenBytes
    ) = 0;

    virtual bool decodeFrame(
        const uint8_t *encoded,
        size_t encodedBytes,
        int16_t *pcm,
        size_t pcmCapacitySamples,
        size_t &writtenSamples
    ) = 0;
};

} // namespace SDR
