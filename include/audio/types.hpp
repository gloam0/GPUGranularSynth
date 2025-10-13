#ifndef GPUGRANULARSYNTH_TYPES_HPP
#define GPUGRANULARSYNTH_TYPES_HPP

#include <array>
#include <cstdint>
#include <cmath>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#include <config.hpp>

struct alignas(64) AudioBlock {
    uint64_t seq = 0;
    uint32_t samples = 0;
    uint32_t channels = config::NUM_CHANNELS;
    // interleaved
    std::array<float, config::GEN_BLOCK_SIZE * config::NUM_CHANNELS> data{};
};

#endif //GPUGRANULARSYNTH_TYPES_HPP