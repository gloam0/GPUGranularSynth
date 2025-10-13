#include <dataflow/spsc_ring_consumer.hpp>

SPSCRingConsumer::SPSCRingConsumer(SPSCRingState& rs, std::vector<AudioBlock>& ring) : rs(rs), ring(ring) {}

void SPSCRingConsumer::start() {
    r_curr = 0;
    slot = 0;
    sample_idx = 0;
    have_block = false;
    try_acquire_block();
}

void SPSCRingConsumer::stop() {
    if (have_block) rs.publish_read(r_curr);
    have_block = false;
}

void SPSCRingConsumer::process(float* const* out, int num_channels, int num_samples) {
    // zero all (extra channels; guard against underruns with no last_good)
    for (int ch = 0; ch < num_channels; ++ch)
        std::fill_n(out[ch], num_samples, 0.0f);

    /*
     * Consume audio blocks from the ring buffer and write them to audio out.
     * The ring buffer and audio out may have different audio block sizes by
     * design (we don't necessarily consume a whole ring buffer slot on each
     * iteration of this callback, nor do we necessarily fill the whole output
     * buffer with one ring buffer slot).
     */
    int i = 0;
    while (i < num_samples) {
        // read if we don't have a block currently; break (safe - zeros in buffer) if
        // underrun and implicit get_last_good() failed.
        if (!have_block) { if (!try_acquire_block()) break; }

        const auto& block = ring[slot];
        const int samples_avail = static_cast<int>(block.samples) - static_cast<int>(sample_idx);
        const int samples_to_copy = std::min(samples_avail, num_samples - i);

        // de-interleave into buffer channels
        for (int n = 0; n < samples_to_copy; ++n) {
            const int already_consumed = (sample_idx + n) * block.channels;
            for (int ch = 0; ch < std::min<int>(block.channels, num_channels); ch++)
                out[ch][i + n] = block.data[already_consumed + ch];
        }

        // update state
        sample_idx += samples_to_copy;
        i += samples_to_copy;

        // if we finished consuming the current block this iter, publish the
        // read and mark have_block false.
        if (sample_idx >= ring[slot].samples) {
            if (have_block) rs.publish_read(r_curr);
            have_block = false;
        }

        // If the block is exactly consumed here, loop will publish on next iteration
    }
}

bool SPSCRingConsumer::try_acquire_block() {
    size_t slot_next{};
    uint64_t r_curr_next{r_curr};
    // non-blocking, if no block ready try get_last_good
    if (!rs.acquire_read(r_curr_next, slot_next)) {
        if (!rs.get_last_good(slot_next)) {
            have_block = false;
            return false;
        }
    }
    slot = slot_next;
    r_curr = r_curr_next;
    sample_idx = 0;
    have_block = true;
    return true;
}
