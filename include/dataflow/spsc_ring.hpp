#ifndef GPUGRANULARSYNTH_SPSC_RING_HPP
#define GPUGRANULARSYNTH_SPSC_RING_HPP

#include <vector>
#include <dataflow/spsc_ring_state.hpp>

template<class T>
class SPSCRing {
public:
    SPSCRing(int N) : rs(N) { ring = std::vector<T>(N); };

    bool try_push(const T& in) {
        size_t slot{};
        uint64_t w{};
        if (!rs.acquire_write(w, slot)) return false;
        ring[slot] = in;
        rs.publish_write(w);
        return true;
    };

    bool try_pop(T& out) {
        size_t slot{};
        uint64_t r{};
        if (!rs.acquire_read(r, slot)) return false;
        out = ring[slot];
        rs.publish_read(r);
        return true;
    };

    int get_underruns() const { return rs.get_underruns(); };
    uint64_t get_read_count() const { return rs.get_read_count(); };
    uint64_t get_write_count() const { return rs.get_write_count(); };

private:
    SPSCRingState rs;
    std::vector<T> ring;
};

#endif //GPUGRANULARSYNTH_SPSC_RING_HPP