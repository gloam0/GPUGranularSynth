#ifndef GPUGRANULARSYNTH_CONFIG_H
#define GPUGRANULARSYNTH_CONFIG_H

#if PROJ_DEBUG
    #define IF_DEBUG(expr) do { expr; } while (0)
#else
    #define IF_DEBUG(expr) ((void)0)
#endif


namespace config {

// Audio
    constexpr int SAMPLE_RATE = 48000;
    constexpr int BLOCK_SIZE = 256;
    // Number of GPU-write-CPU-read buffers in the ring (>1), e.g., 3 for triple-buffering,
    // induces BUFFER_RING_SIZE * BLOCK_SIZE / SAMPLE_RATE constant latency
    constexpr int BUFFER_RING_SIZE = 3;
// Audio (non-configurable)
    constexpr int NUM_CHANNELS = 2;
// CUDA
    constexpr int CUDA_SELECTED_DEVICE = 0;

}

#endif //GPUGRANULARSYNTH_CONFIG_H