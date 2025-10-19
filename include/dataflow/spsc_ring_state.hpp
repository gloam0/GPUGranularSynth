#ifndef GPUGRANULARSYNTH_SPSC_RING_STATE_HPP
#define GPUGRANULARSYNTH_SPSC_RING_STATE_HPP

#include <cstdint>
#include <atomic>

/*
* Thread-safe state tracking for a Single Producer, Single Consumer ring.
* Does not own the ring.
*/
class SPSCRingState {
public:
    explicit SPSCRingState(const size_t slots) : slots(slots) {}

    // Check if ready to write, write to returned 'slot' idx
    bool acquire_write(uint64_t& w_curr, size_t& slot) const {
        const auto r = read_count.load(std::memory_order_acquire);
        const auto w = write_count.load(std::memory_order_relaxed);
        if (w - r >= slots) return false; // full, w is N ahead of r
        w_curr = w;
        slot = w % slots;
        return true;
    }

    // acquire_write() -> <do_write(slot)> -> publish_write()
    void publish_write(const uint64_t w_curr) {
        write_count.store(w_curr + 1, std::memory_order_release);
    }

    // Check if ready to read, read from returned 'slot' idx
    bool acquire_read(uint64_t& r_curr, size_t& slot) {
        const auto w = write_count.load(std::memory_order_acquire);
        const auto r = read_count.load(std::memory_order_relaxed);
        if (r == w) { underruns++; return false; } // empty
        r_curr = r;
        slot = r % slots;
        return true;
    }

    // acquire_read() -> <do_read(slot)> -> publish_read()
    void publish_read(const uint64_t r_curr) {
        read_count.store(r_curr + 1, std::memory_order_release);
    }

    // Get idx of last completed read
    // Call if a read is needed but acquire_read() == false
    bool get_last_good(size_t& slot) const {
        const auto r = read_count.load(std::memory_order_relaxed);
        if (r == 0) return false;
        slot = (r - 1) % slots;
        return true;
    }

    int get_underruns() const { return underruns; };
    uint64_t get_read_count() const { return read_count.load(std::memory_order_acquire); };
    uint64_t get_write_count() const { return write_count.load(std::memory_order_acquire); };

    // monotonically increasing write/read counts
    // alignas(64) to prevent cache invalidations
    alignas(64) std::atomic<uint64_t> write_count{0};
    alignas(64) std::atomic<uint64_t> read_count{0};
    int underruns = 0; // non-atomic, strictly SPSC
    const size_t slots;
};

#endif //GPUGRANULARSYNTH_SPSC_RING_STATE_HPP