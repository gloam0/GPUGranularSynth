#ifndef GPUGRANULARSYNTH_SPSC_RING_HPP
#define GPUGRANULARSYNTH_SPSC_RING_HPP

#include <cstdint>
#include <atomic>

/*
* Thread-safe state tracking for a Single Producer, Single Consumer ring.
* Does not own the ring.
*/
class SPSCRingState {
public:
    explicit SPSCRingState(size_t slots);

    // Check if ready to write, write to returned 'slot' idx
    bool acquire_write(uint64_t& w_curr, size_t& slot) const;

    // acquire_write() -> <do_write(slot)> -> publish_write()
    void publish_write(uint64_t w_curr);

    // Check if ready to read, read from returned 'slot' idx
    bool acquire_read(uint64_t& r_curr, size_t& slot);

    // acquire_read() -> <do_read(slot)> -> publish_read()
    void publish_read(uint64_t r_curr);

    // Get idx of last completed read
    // Call if a read is needed but acquire_read() == false
    bool get_last_good(size_t& slot) const;

    int get_underruns() const;

    uint64_t get_read_count() const;

    uint64_t get_write_count() const;

    // monotonically increasing write/read counts
    // alignas(64) to prevent cache invalidations
    alignas(64) std::atomic<uint64_t> write_count{0};
    alignas(64) std::atomic<uint64_t> read_count{0};
    int underruns = 0; // non-atomic, strictly SPSC
    const size_t slots;
};

#endif //GPUGRANULARSYNTH_SPSC_RING_HPP