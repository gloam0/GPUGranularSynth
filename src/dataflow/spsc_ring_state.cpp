#include <dataflow/spsc_ring_state.hpp>

SPSCRingState::SPSCRingState(const size_t slots) : slots(slots) {}

// Check if ready to write, write to returned 'slot' idx
bool SPSCRingState::acquire_write(uint64_t& w_curr, size_t& slot) const {
    const auto r = read_count.load(std::memory_order_acquire);
    const auto w = write_count.load(std::memory_order_relaxed);
    if (w - r >= slots) return false; // full, w is N ahead of r
    w_curr = w;
    slot = w % slots;
    return true;
}

// acquire_write() -> <do_write(slot)> -> publish_write()
void SPSCRingState::publish_write(const uint64_t w_curr) {
    write_count.store(w_curr + 1, std::memory_order_release);
}

// Check if ready to read, read from returned 'slot' idx
bool SPSCRingState::acquire_read(uint64_t& r_curr, size_t& slot) {
    const auto w = write_count.load(std::memory_order_acquire);
    const auto r = read_count.load(std::memory_order_relaxed);
    if (r == w) { underruns++; return false; } // empty
    r_curr = r;
    slot = r % slots;
    return true;
}

// acquire_read() -> <do_read(slot)> -> publish_read()
void SPSCRingState::publish_read(const uint64_t r_curr) {
    read_count.store(r_curr + 1, std::memory_order_release);
}

// Get idx of last completed read
// Call if a read is needed but acquire_read() == false
bool SPSCRingState::get_last_good(size_t& slot) const {
    const auto r = read_count.load(std::memory_order_relaxed);
    if (r == 0) return false;
    slot = (r - 1) % slots;
    return true;
}

int SPSCRingState::get_underruns() const { return underruns; };
