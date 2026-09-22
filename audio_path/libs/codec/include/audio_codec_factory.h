#pragma once

#include <memory>

#include "codec_interface.h"

namespace SDR {

class AudioCodecFactory {
 public:
  static std::unique_ptr<AudioCodec> create(AudioCodecId id,
                                            AudioCodecConfig config = {});
};

}  // namespace SDR