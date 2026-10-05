#include "include/ima_adpcm_codec.h"

#include <algorithm>
#include <cstdint>

#include "git_info.h"

namespace SDR {

namespace {

constexpr size_t ADPCM_BLOCK_HEADER_BYTES = 4;

struct ImaState {
  int predictor;
  int index;
};

constexpr int INDEX_TABLE[16] = {-1, -1, -1, -1, 2, 4, 6, 8,
                                 -1, -1, -1, -1, 2, 4, 6, 8};

constexpr int STEP_TABLE[89] = {
    7,     8,     9,     10,    11,    12,    13,    14,    16,    17,
    19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
    50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
    130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
    337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
    876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
    2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
    5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767};

int clampInt(int value, int low, int high) {
  return std::max(low, std::min(value, high));
}

void putLe16(uint8_t* p, int16_t value) {
  uint16_t u = static_cast<uint16_t>(value);

  p[0] = static_cast<uint8_t>(u & 0xFF);
  p[1] = static_cast<uint8_t>((u >> 8) & 0xFF);
}

int16_t getLe16(const uint8_t* p) {
  uint16_t u = static_cast<uint16_t>(p[0]) |
               static_cast<uint16_t>(static_cast<uint16_t>(p[1]) << 8);

  return static_cast<int16_t>(u);
}

uint8_t encodeNibble(int16_t sample, ImaState& state) {
  int step = STEP_TABLE[state.index];
  int diff = static_cast<int>(sample) - state.predictor;

  uint8_t code = 0;

  if (diff < 0) {
    code |= 0x08;
    diff = -diff;
  }

  int delta = step >> 3;

  if (diff >= step) {
    code |= 0x04;
    diff -= step;
    delta += step;
  }

  if (diff >= (step >> 1)) {
    code |= 0x02;
    diff -= (step >> 1);
    delta += (step >> 1);
  }

  if (diff >= (step >> 2)) {
    code |= 0x01;
    delta += (step >> 2);
  }

  if (code & 0x08) {
    state.predictor -= delta;
  } else {
    state.predictor += delta;
  }

  state.predictor = clampInt(state.predictor, -32768, 32767);

  state.index += INDEX_TABLE[code & 0x0F];
  state.index = clampInt(state.index, 0, 88);

  return code & 0x0F;
}

int16_t decodeNibble(uint8_t code, ImaState& state) {
  code &= 0x0F;

  int step = STEP_TABLE[state.index];
  int delta = step >> 3;

  if (code & 0x04) {
    delta += step;
  }

  if (code & 0x02) {
    delta += step >> 1;
  }

  if (code & 0x01) {
    delta += step >> 2;
  }

  if (code & 0x08) {
    state.predictor -= delta;
  } else {
    state.predictor += delta;
  }

  state.predictor = clampInt(state.predictor, -32768, 32767);

  state.index += INDEX_TABLE[code];
  state.index = clampInt(state.index, 0, 88);

  return static_cast<int16_t>(state.predictor);
}

}  // anonymous namespace

ImaAdpcmCodec::ImaAdpcmCodec(AudioCodecConfig config) : config_(config) {}

AudioCodecId ImaAdpcmCodec::id() const {
  return AudioCodecId::IMA_ADPCM;
}

const char* ImaAdpcmCodec::name() const {
  return "IMA_ADPCM";
}

uint32_t ImaAdpcmCodec::sampleRate() const {
  return config_.sampleRate;
}

size_t ImaAdpcmCodec::frameSamples() const {
  return config_.frameSamples;
}

uint8_t ImaAdpcmCodec::channels() const {
  return config_.channels;
}

size_t ImaAdpcmCodec::encodedBytesForFrame() const {
  if (config_.frameSamples == 0) {
    return 0;
  }

  const size_t adpcmNibbles = config_.frameSamples - 1;
  const size_t adpcmDataBytes = (adpcmNibbles + 1) / 2;

  return ADPCM_BLOCK_HEADER_BYTES + adpcmDataBytes;
}

bool ImaAdpcmCodec::encodeFrame(const int16_t* pcm, size_t pcmSamples,
                                uint8_t* encoded, size_t encodedCapacity,
                                size_t& writtenBytes) {
  writtenBytes = 0;

  if (pcm == nullptr || encoded == nullptr) {
    return false;
  }

  if (config_.channels != 1) {
    return false;
  }

  if (config_.frameSamples == 0) {
    return false;
  }

  if (pcmSamples != config_.frameSamples) {
    return false;
  }

  const size_t requiredBytes = encodedBytesForFrame();

  if (encodedCapacity < requiredBytes) {
    return false;
  }

  /*
   * ADPCM block format:
   *
   * byte 0..1 : initial predictor / first PCM sample, little-endian int16
   * byte 2    : initial step index
   * byte 3    : reserved
   * byte 4..  : packed ADPCM nibbles
   *
   * This makes every ADPCM frame independently decodable.
   * That is useful for packets because one lost packet does not destroy
   * the decoder state forever.
   */

  putLe16(&encoded[0], pcm[0]);
  encoded[2] = 0;
  encoded[3] = 0;

  ImaState state{};
  state.predictor = pcm[0];
  state.index = 0;

  size_t outIndex = ADPCM_BLOCK_HEADER_BYTES;
  bool haveLowNibble = false;
  uint8_t packedByte = 0;

  for (size_t i = 1; i < pcmSamples; i++) {
    uint8_t code = encodeNibble(pcm[i], state);

    if (!haveLowNibble) {
      packedByte = code;
      haveLowNibble = true;
    } else {
      packedByte |= static_cast<uint8_t>(code << 4);
      encoded[outIndex++] = packedByte;
      packedByte = 0;
      haveLowNibble = false;
    }
  }

  if (haveLowNibble) {
    encoded[outIndex++] = packedByte;
  }

  writtenBytes = outIndex;
  return writtenBytes == requiredBytes;
}

bool ImaAdpcmCodec::decodeFrame(const uint8_t* encoded, size_t encodedBytes,
                                int16_t* pcm, size_t pcmCapacitySamples,
                                size_t& writtenSamples) {
  writtenSamples = 0;

  if (encoded == nullptr || pcm == nullptr) {
    return false;
  }

  if (config_.channels != 1) {
    return false;
  }

  if (config_.frameSamples == 0) {
    return false;
  }

  const size_t requiredBytes = encodedBytesForFrame();

  if (encodedBytes != requiredBytes) {
    return false;
  }

  if (pcmCapacitySamples < config_.frameSamples) {
    return false;
  }

  ImaState state{};
  state.predictor = getLe16(&encoded[0]);
  state.index = clampInt(encoded[2], 0, 88);

  pcm[0] = static_cast<int16_t>(state.predictor);

  for (size_t i = 1; i < config_.frameSamples; i++) {
    const size_t byteIndex = ADPCM_BLOCK_HEADER_BYTES + ((i - 1) / 2);
    const uint8_t packedByte = encoded[byteIndex];

    uint8_t code;

    if (((i - 1) & 1U) == 0) {
      code = packedByte & 0x0F;
    } else {
      code = (packedByte >> 4) & 0x0F;
    }

    pcm[i] = decodeNibble(code, state);
  }

  writtenSamples = config_.frameSamples;
  return true;
}

}  // namespace SDR