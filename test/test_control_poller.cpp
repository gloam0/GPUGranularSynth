#include <doctest.h>
#include <thread>
#include <controls/control_poller.hpp>

TEST_CASE("ControlPoller: Check duplicates ignored") {
    ControlSurface cs{128};
    for (auto& c : cs.controls)
        c.store(1.f, std::memory_order_relaxed);
    SPSCRing<ControlSample> ring(128);
    ControlPoller poller(cs, ring, 200);
    poller.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    poller.stop();

    ControlSample v;
    CHECK(!ring.try_pop(v));
}

TEST_CASE("ControlPoller: Verify value change propagates") {
    ControlSurface cs{128};
    for (auto& c : cs.controls)
        c.store(1.f, std::memory_order_relaxed);
    SPSCRing<ControlSample> ring(1024);
    ControlPoller poller(cs, ring, 200);
    poller.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const uint64_t id = 123;
    const float val = 0.5f;
    cs.controls[id].store(val, std::memory_order_relaxed);
    std::atomic<bool> seen{false};
    std::atomic<bool> stop{false};
    std::thread read([&] {
        while (!seen.load(std::memory_order_relaxed)
            && !stop.load(std::memory_order_relaxed)) {
            ControlSample v;
            if (!ring.try_pop(v)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            if (v.control_id == id && std::fabs(v.value - val) < 0.001f)
                seen.store(true, std::memory_order_release);
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    stop.store(true, std::memory_order_release);
    read.join();
    CHECK(seen.load(std::memory_order_relaxed));
}