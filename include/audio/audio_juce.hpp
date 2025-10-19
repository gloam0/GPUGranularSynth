#ifndef GPUGRANULARSYNTH_AUDIO_JUCE_H
#define GPUGRANULARSYNTH_AUDIO_JUCE_H

#include <juce_audio_devices/juce_audio_devices.h>

#include <dataflow/spsc_ring_audio_blocks.hpp>

bool juce_dm_setup(juce::AudioDeviceManager& dm);

inline std::atomic<uint64_t> dev_sample_count{0};

inline uint64_t get_dev_sample_count() noexcept {
    return dev_sample_count.load(std::memory_order_acquire);
}

class SPSCRingPlayer : public juce::AudioIODeviceCallback {
public:
    SPSCRingPlayer(SPSCRingState& rs, std::vector<AudioBlock>& ring)
        : rc(rs, ring) {}

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
    }

private:
    SPSCRingAudioBlocks rc;
};

#endif //GPUGRANULARSYNTH_AUDIO_JUCE_H