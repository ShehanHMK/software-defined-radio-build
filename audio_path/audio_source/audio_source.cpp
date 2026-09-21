#include <portaudio.h>

#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>

static constexpr int SAMPLE_RATE = 8000;
static constexpr int FRAME_SAMPLES = 160;

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

/*
Audio Frame details

    * Sampling period = 1/8000 = 0.125ms
    * Number of samples per frame = 160
    * Frame size (in time) = 160 x 0.125ms = 20ms
    * PCM bits per sample = 16 (16bits, 2bytes)
    * Frame size (in bits/bytes) = 160 x 2 (bytes) = 320bytes
    * ADPCM compression ration -> 4:1 (compresses 16bit sample to 4bit sample) 
    * Output frame size from ADPCM = 80bytes
*/

int main() {
    std::cout << "Audio source application." << std::endl;

    return 0;
}

