#include <portaudio.h>

#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>

static constexpr int SAMPLE_RATE = 8000;
static constexpr int FRAME_SAMPLES = 160; // 20ms frame at 8khz. 

static const int STEP_TABLE[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const int INDEX_TABLE[16] = {
    -1, -1, -1, -1,
     2,  4,  6,  8,
    -1, -1, -1, -1,
     2,  4,  6,  8
};

static int clampInt(int value, int minValue, int maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}

struct AdpcmState {
    int predictor = 0;
    int index = 0;
};

uint8_t encodeSample(int16_t sample, AdpcmState& state) {
    int step = STEP_TABLE[state.index];
    int diff = sample - state.predictor;

    uint8_t code = 0;

    if (diff < 0) {
        code |= 8;
        diff = -diff;
    }

    int delta = step >> 3;

    if (diff >= step) {
        code |= 4;
        diff -= step;
        delta += step;
    }

    if (diff >= (step >> 1)) {
        code |= 2;
        diff -= (step >> 1);
        delta += (step >> 1);
    }

    if (diff >= (step >> 2)) {
        code |= 1;
        delta += (step >> 2);
    }

    if (code & 8) {
        state.predictor -= delta;
    } else {
        state.predictor += delta;
    }

    state.predictor = clampInt(state.predictor, -32768, 32767);

    state.index += INDEX_TABLE[code];
    state.index = clampInt(state.index, 0, 88);

    return code & 0x0F;
}

int16_t decodeNibble(uint8_t code, AdpcmState& state) {
    int step = STEP_TABLE[state.index];
    int delta = step >> 3;

    if (code & 4) {
        delta += step;
    }

    if (code & 2) {
        delta += step >> 1;
    }

    if (code & 1) {
        delta += step >> 2;
    }

    if (code & 8) {
        state.predictor -= delta;
    } else {
        state.predictor += delta;
    }

    state.predictor = clampInt(state.predictor, -32768, 32767);

    state.index += INDEX_TABLE[code];
    state.index = clampInt(state.index, 0, 88);

    return static_cast<int16_t>(state.predictor);
}

std::vector<uint8_t> encodeFrame(const std::vector<int16_t>& pcm) {
    AdpcmState state;

    std::vector<uint8_t> encoded;
    encoded.reserve((pcm.size() + 1) / 2);

    bool haveLowNibble = false;
    uint8_t lowNibble = 0;

    for (int16_t sample : pcm) {
        uint8_t code = encodeSample(sample, state);

        if (!haveLowNibble) {
            lowNibble = code;
            haveLowNibble = true;
        } else {
            uint8_t packed = lowNibble | (code << 4);
            encoded.push_back(packed);
            haveLowNibble = false;
        }
    }

    if (haveLowNibble) {
        encoded.push_back(lowNibble);
    }

    return encoded;
}

std::vector<int16_t> decodeFrame(const std::vector<uint8_t>& encoded, size_t expectedSamples) {
    AdpcmState state;

    std::vector<int16_t> decoded;
    decoded.reserve(expectedSamples);

    for (uint8_t b : encoded) {
        uint8_t low = b & 0x0F;
        uint8_t high = (b >> 4) & 0x0F;

        decoded.push_back(decodeNibble(low, state));

        if (decoded.size() < expectedSamples) {
            decoded.push_back(decodeNibble(high, state));
        }
    }

    return decoded;
}

int main() {
    PaError err = Pa_Initialize();

    if (err != paNoError) {
        std::cerr << "PortAudio init failed: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    PaStream* stream = nullptr;

    err = Pa_OpenDefaultStream(
        &stream,
        1,              // input channels: mono mic
        1,              // output channels: mono speaker
        paInt16,        // 16-bit PCM
        SAMPLE_RATE,
        FRAME_SAMPLES,
        nullptr,
        nullptr
    );

    if (err != paNoError) {
        std::cerr << "Failed to open audio stream: " << Pa_GetErrorText(err) << std::endl;
        std::cerr << "Try changing SAMPLE_RATE from 8000 to 16000." << std::endl;
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);

    if (err != paNoError) {
        std::cerr << "Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Live mic -> ADPCM encode -> decode -> speaker started." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    std::vector<int16_t> input(FRAME_SAMPLES);
    std::vector<int16_t> output;

    uint64_t frameCount = 0;

    while (true) {
        err = Pa_ReadStream(stream, input.data(), FRAME_SAMPLES);

        if (err == paInputOverflowed) {
            std::cerr << "Warning: input overflow, continuing..." << std::endl;
            continue;
        }

        if (err != paNoError) {
            std::cerr << "Read error: " << Pa_GetErrorText(err) << std::endl;
            break;
        }

        std::vector<uint8_t> encoded = encodeFrame(input);
        output = decodeFrame(encoded, input.size());

        err = Pa_WriteStream(stream, output.data(), FRAME_SAMPLES);

        if (err == paOutputUnderflowed) {
            std::cerr << "Warning: output underflow, continuing..." << std::endl;
            continue;
        }

        if (err != paNoError) {
            std::cerr << "Write error: " << Pa_GetErrorText(err) << std::endl;
            break;
        }

        frameCount++;

        if (frameCount % 50 == 0) {
            std::cout << "Frame " << frameCount
                      << " | raw=" << input.size() * sizeof(int16_t)
                      << " bytes"
                      << " | encoded=" << encoded.size()
                      << " bytes"
                      << std::endl;
        }
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
