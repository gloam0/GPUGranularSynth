#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <config.hpp>
#include <dataflow/spsc_ring_state.hpp>

int main() {
    SPSCRingState rs(config::BUFFER_RING_SIZE);

    std::vector<uint64_t> ring(config::BUFFER_RING_SIZE, 0);
    int n = 0;

    // 'startup' - pre-process a half ring before starting consumer
    for (int i = 0; i < (config::BUFFER_RING_SIZE / 2); i++) {
        uint64_t w_curr{};
        size_t slot{};
        while (!rs.acquire_write(w_curr, slot)) {/*e.g., x86's PAUSE ?*/}
        // Audio processing would happen here, writing directly to ring[slot] {
        ring[slot] = n;
        n++;
        // }
        rs.publish_write(w_curr);
    }

    std::thread prod([&] {
        uint64_t w_curr{};
        size_t slot{};
        for (;;) {
            while (!rs.acquire_write(w_curr, slot)) {/*e.g., x86's PAUSE ?*/}
            // Audio processing would happen here, writing directly to ring[slot] {
            std::cout << "[P] wrote value=" << n
                      << " -> slot=" << slot
                      << " (w_curr=" << w_curr << ")";
            ring[slot] = n;
            n++;
            // }
            rs.publish_write(w_curr);
        }
    });

    std::thread cons([&] {
        uint64_t r_curr{};
        size_t slot{};
        for (;;) {
            if (!rs.acquire_read(r_curr, slot)) {
                rs.get_last_good(slot);
                continue;
            }
            // Read happens here when the next audio block is needed {
            std::cout << "    [C] read value=" << ring[slot]
                      << " <- slot=" << slot
                      << " (r_curr=" << r_curr << ")\n";
            // }
            rs.publish_read(r_curr);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    prod.join();
    cons.join();
    return 0;
}