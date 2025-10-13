#ifndef GPUGRANULARSYNTH_CONFIG_H
#define GPUGRANULARSYNTH_CONFIG_H

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#if PROJ_DEBUG
    #define IF_DEBUG(expr) do { expr; } while (0)
    #define IF_RELEASE(expr) ((void)0)
#else
    #define IF_DEBUG(expr) ((void)0)
    #define IF_RELEASE(expr) do { expr; } while (0)
#endif


namespace config {

// Audio
    inline int SAMPLE_RATE = 48000;  // non-const, sample rate can technically change at runtime
    constexpr int GEN_BLOCK_SIZE = 1024;
    inline int DEV_BLOCK_SIZE = 128;
    // Number of GPU-write-CPU-read buffers in the ring (>1), e.g., 3 for triple-buffering,
    // induces BUFFER_RING_SIZE * BLOCK_SIZE / SAMPLE_RATE constant latency
    constexpr int BUFFER_RING_SIZE = 3;
// Audio (non-configurable)
    constexpr int NUM_CHANNELS = 2;
// CUDA
    constexpr int CUDA_SELECTED_DEVICE = 0;

// Helpers
    inline double block_latency_ms() noexcept {
        // duration (ms) of one generator block
        return 1000.0 * static_cast<double>(GEN_BLOCK_SIZE) / static_cast<double>(SAMPLE_RATE);
    }
    inline double ring_latency_ms() noexcept {
        // total pipeline latency (ms) of ring
        return block_latency_ms() * static_cast<double>(BUFFER_RING_SIZE);
    }

}

#endif //GPUGRANULARSYNTH_CONFIG_H