#include <algorithm>
#include <thread>
#include <controls/control_poller.hpp>
#include <audio/audio_juce.hpp>

ControlPoller::ControlPoller(ControlSurface &cs, SPSCRing<ControlSample>& ring, int rate_hz)
    : cs(cs), cs_size(cs.controls.size()), cs_curr(cs_size), cs_prev(cs_size), ring(ring), rate(rate_hz),
      period(std::chrono::duration_cast<_clock::duration>(std::chrono::duration<double>(1.0 / rate_hz)))
{ snapshot(cs_prev); }

ControlPoller::~ControlPoller() { stop(); }

void ControlPoller::start() {
    bool expected = false;
    if (!run_flag.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) return;
    poll_thread = std::thread([this]{ thread_main(); });
}

void ControlPoller::stop() {
    bool expected = true;
    if (run_flag.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
        if (poll_thread.joinable()) poll_thread.join();
    }
}

void ControlPoller::thread_main() {
    auto deadline = _clock::now() + period;
    while (run_flag.load(std::memory_order_acquire)) {
        auto now = _clock::now();

        while (deadline <= now) deadline += period;
        std::this_thread::sleep_until(deadline);

        snapshot(cs_curr);
        const uint64_t t = get_dev_sample_count();
        for (uint16_t i = 0; i < cs_size; i++) {
            float v_curr = std::clamp(cs_curr[i], 0.f, 1.f);
            const float v_prev = cs_prev[i];

            if (v_curr != v_prev) {
                ControlSample sample{t, v_curr, i};
                if (ring.try_push(sample)) cs_prev[i] = v_curr;
            }
        }
        deadline += period;
    }
}

void ControlPoller::snapshot(std::vector<float>& dst) const {
    for (uint32_t i = 0; i < cs_size; ++i)
        dst[i] = cs.controls[i].load(std::memory_order_acquire);
}