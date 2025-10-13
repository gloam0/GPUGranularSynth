#ifndef GPUGRANULARSYNTH_TYPES_HPP
#define GPUGRANULARSYNTH_TYPES_HPP

#include <array>
#include <cstdint>
#include <cmath>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#include <config.hpp>

/*
 * channel-major audio block
 */
struct alignas(64) AudioBlock {
    uint32_t samples = 0;
    static constexpr uint32_t channels = config::NUM_CHANNELS;
    std::array<float, config::GEN_BLOCK_SIZE * config::NUM_CHANNELS> data{};

    inline float* ch(uint32_t ch) noexcept {
        return data.data() + ch * config::GEN_BLOCK_SIZE;
    }

    inline const float* ch(uint32_t ch) const noexcept {
        return data.data() + ch * config::GEN_BLOCK_SIZE;
    }
};

#endif //GPUGRANULARSYNTH_TYPES_HPP