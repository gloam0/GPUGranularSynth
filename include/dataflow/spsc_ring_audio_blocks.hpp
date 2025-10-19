#ifndef GPUGRANULARSYNTH_SPSC_RING_CONSUMER_HPP
#define GPUGRANULARSYNTH_SPSC_RING_CONSUMER_HPP

#include <vector>
#include <dataflow/spsc_ring_state.hpp>
#include <audio/types.hpp>

class SPSCRingConsumer {
public:
    SPSCRingConsumer(SPSCRingState& rs, std::vector<AudioBlock>& ring);

    void start();
    void stop();
    // copies directly from ring to out
    void consume(float* const* out, int num_channels, int num_samples);

private:
    bool try_acquire_block();

    SPSCRingState& rs;
    std::vector<AudioBlock>& ring;

    uint64_t r_curr{0};              // must propagate r_curr for SPSCRingState
    size_t   slot{0};                // slot of current read from SPSCRingState
    uint32_t sample_idx{0};          // idx of first not-yet-consumed sample in current block
    bool     have_block{false};      // true if current block hasn't been fully consumed
    bool     block_acquired{false};  // true if current block is new (i.e., not a get_last_good() fallback)
};

#endif //GPUGRANULARSYNTH_SPSC_RING_CONSUMER_HPP