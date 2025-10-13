#include <array>
#include <iostream>
#include <chrono>
#include <config.hpp>
#include <utils/cuda_dev_info.hpp>

#include <dataflow/spsc_ring_state.hpp>
#include <audio/audio_juce.hpp>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

struct DemoSineState {
    double phase = 0.0;
    double phase_delta_right = 0.0;

    double phase_inc = 0.0;

    void init(double freq, double sample_rate, double phase_delta_r) {
        phase_inc = 2.0 * M_PI * freq / sample_rate;
        phase_delta_right = phase_delta_r;
    }
};

static inline void generate_stereo_sine_block(DemoSineState& state, AudioBlock& block, double amp) {
    block.samples = static_cast<uint32_t>(config::GEN_BLOCK_SIZE);

    float* out = block.data.data();
    for (size_t i = 0; i < config::GEN_BLOCK_SIZE; ++i) {
        out[i] = static_cast<float>(amp * std::sin(state.phase));
        out[config::GEN_BLOCK_SIZE + i] = static_cast<float>(amp * std::sin(state.phase + state.phase_delta_right));

        state.phase += state.phase_inc;
        if (state.phase >= 2.0 * M_PI) state.phase -= 2.0 * M_PI;
        else if (state.phase < 0.0)    state.phase += 2.0 * M_PI;
    }
}

int main() {
    int dev_count = cuda_get_device_count();
    int dev_use = (dev_count > 1) ? config::CUDA_SELECTED_DEVICE : 0;
    cudaDeviceProp dev_use_prop = cuda_get_device_properties(0);

    cudaSetDevice(dev_use);
    cuda_print_device_info(dev_use, &dev_use_prop);

    // JUCE device setup
    juce::ScopedJuceInitialiser_GUI juce_init;

    juce::AudioDeviceManager dm;
    if (!juce_dm_setup(dm)) { std::cout << "\nAudioDeviceManager setup failed.\n"; return 1; }

    // SPSCRing
    SPSCRingState rs(config::BUFFER_RING_SIZE);
    std::vector<AudioBlock> ring(config::BUFFER_RING_SIZE);

    // demo sine generator
    DemoSineState sine_state{};
    sine_state.init(220, config::SAMPLE_RATE, M_PI / 2.0);

    // Producer thread (fills when space is available)
    std::thread prod([&]{
        uint64_t w_curr{};
        size_t slot{};
        for (;;) {
            while (!rs.acquire_write(w_curr, slot)) {/* spin */}
            auto& block = ring[slot];
            generate_stereo_sine_block(sine_state, block, 0.2);
            rs.publish_write(w_curr);
        }
    });

    // Create player, start consuming from audio callback
    SPSCRingPlayer player{ rs, ring };
    dm.addAudioCallback(&player);

    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return 0;
}