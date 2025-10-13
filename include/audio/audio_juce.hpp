#ifndef GPUGRANULARSYNTH_AUDIO_JUCE_H
#define GPUGRANULARSYNTH_AUDIO_JUCE_H

#include <juce_audio_devices/juce_audio_devices.h>

#include <dataflow/spsc_ring_consumer.hpp>

bool juce_dm_setup(juce::AudioDeviceManager& dm);

class SPSCRingPlayer : public juce::AudioIODeviceCallback {
public:
    SPSCRingPlayer(SPSCRingState& rs, std::vector<AudioBlock>& ring)
        : consumer(rs, ring) {}

    void audioDeviceAboutToStart(juce::AudioIODevice*) override { consumer.start(); }
    void audioDeviceStopped() override { consumer.stop(); }

    void audioDeviceIOCallbackWithContext(const float* const*, int,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext&) override
    {
        juce::ScopedNoDenormals no_denormals;
        consumer.process(outputChannelData, numOutputChannels, numSamples);
    }

private:
    SPSCRingConsumer consumer;
};

#endif //GPUGRANULARSYNTH_AUDIO