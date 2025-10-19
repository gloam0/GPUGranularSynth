#ifndef GPUGRANULARSYNTH_AUDIO_JUCE_H
#define GPUGRANULARSYNTH_AUDIO_JUCE_H

#include <juce_audio_devices/juce_audio_devices.h>

#include <dataflow/spsc_ring_audio_blocks.hpp>
#include <utils/utils.hpp>

bool juce_dm_setup(juce::AudioDeviceManager& dm);

inline std::atomic<uint64_t> dev_sample_count{0};

inline uint64_t get_dev_sample_count() noexcept {
    return dev_sample_count.load(std::memory_order_relaxed);
}

struct SampleClock {
    explicit SampleClock(const double sr) : sr(sr) {};

    std::atomic<double> sr{48000.0};
    std::atomic<uint64_t> sample_count{0};
    std::atomic<int64_t> time_ns{0};

    uint64_t est_now_samples() const noexcept {
        const auto t0 = time_ns.load(std::memory_order_acquire);
        const auto s0 = sample_count.load(std::memory_order_acquire);
        const double rate = sr.load(std::memory_order_acquire);
        const int64_t dt = now_steady_ns() - t0;
        const double inc = dt > 0 ? static_cast<double>(dt) * rate / 1e9 : 0.;
        return s0 + static_cast<uint64_t>(inc);
    }

    void update() {
        sample_count.store(get_dev_sample_count(), std::memory_order_release);
        time_ns.store(now_steady_ns(), std::memory_order_release);
    }
};

class SPSCRingPlayer : public juce::AudioIODeviceCallback {
public:
    SPSCRingPlayer(SPSCRingState& rs, std::vector<AudioBlock>& ring, SampleClock& sc)
        : rc(rs, ring), sc(sc) {}

    void audioDeviceAboutToStart(juce::AudioIODevice*) override { rc.start(); }
    void audioDeviceStopped() override { rc.stop(); }

    void audioDeviceIOCallbackWithContext(const float* const*, int,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext&) override
    {
        juce::ScopedNoDenormals no_denormals;
        if (numSamples != config::DEV_BLOCK_SIZE) { std::abort(); }  // TODO: re-init on device block size change
        rc.consume(outputChannelData, numOutputChannels, numSamples);
        dev_sample_count.fetch_add(static_cast<uint64_t>(numSamples), std::memory_order_release);
        sc.update();
    }

private:
    SPSCRingAudioBlocks rc;
    SampleClock& sc;
};

#endif //GPUGRANULARSYNTH_AUDIO_JUCE_H