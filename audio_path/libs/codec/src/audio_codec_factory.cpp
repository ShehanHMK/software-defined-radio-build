#include "include/audio_codec_factory.h"

#include "include/ima_adpcm_codec.h"

namespace SDR {

std::unique_ptr<AudioCodec> AudioCodecFactory::create(AudioCodecId id,
                                                      AudioCodecConfig config) {
  switch (id) {
    case AudioCodecId::PCM16:
      // return std::unique_ptr<AudioCodec>(
      //     new Pcm16Codec(config)
      // );
      return std::unique_ptr<AudioCodec>(new ImaAdpcmCodec(config));
    case AudioCodecId::IMA_ADPCM:
      return std::unique_ptr<AudioCodec>(new ImaAdpcmCodec(config));

    default:
      return std::unique_ptr<AudioCodec>(new ImaAdpcmCodec(config));
  }
}

}  // namespace SDR