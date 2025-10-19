#ifndef GPUGRANULARSYNTH_CONTROL_POLLER_HPP
#define GPUGRANULARSYNTH_CONTROL_POLLER_HPP

#include <atomic>
#include <vector>
#include <chrono>
#include <dataflow/spsc_ring.hpp>

using _clock = std::chrono::steady_clock;

// control-owning thread updates this table at time of control change
struct ControlSurface {
    ControlSurface(int n) : controls(n) {};
    std::vector<std::atomic<float>> controls;
};

// sample of a control in the ControlSurface at sample time t
struct ControlSample {
    uint64_t t;
    float value;  // [0.0, 1.0]
    uint16_t control_id;
};

// polls ControlSurface periodically, pushing ControlSamples to SPSC queue for controls which changed
class ControlPoller {
public:
    ControlPoller(ControlSurface& cs, SPSCRing<ControlSample>& ring, int rate_hz);
    ~ControlPoller();

    void start();
    void stop();

private:
    void thread_main();
    void snapshot(std::vector<float>& snapshot) const;

    ControlSurface& cs;
    size_t cs_size;
    std::vector<float> cs_curr;
    std::vector<float> cs_prev;
    SPSCRing<ControlSample>& ring;

    std::thread poll_thread;
    std::atomic<bool> run_flag{false};

    int rate;
    _clock::duration period;
};

#endif //GPUGRANULARSYNTH_CONTROL_POLLER_HPP