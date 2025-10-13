#include <config.hpp>
#include <audio/audio_juce.hpp>

bool juce_dm_setup(juce::AudioDeviceManager& dm) {
    // use system default device
    juce::String audioError = dm.initialise(0, config::NUM_CHANNELS, nullptr, true);
    if (audioError.isNotEmpty()) { std::cerr << "Audio init error: " << audioError << "\n"; return false; }

    // configure
    juce::AudioDeviceManager::AudioDeviceSetup dm_setup;
    dm.getAudioDeviceSetup(dm_setup);
    dm_setup.sampleRate = static_cast<double>(config::SAMPLE_RATE);
    dm_setup.bufferSize = config::DEV_BLOCK_SIZE;
    audioError = dm.setAudioDeviceSetup(dm_setup, true);
    if (audioError.isNotEmpty()) std::cerr << "Audio setup warning/error: " << audioError << "\n";

    // requested configuration may not be supported by the device, update
    // config with the device's active sample rate and buffer size
    if (auto* dev = dm.getCurrentAudioDevice()) {
        std::cout << "Selected audio device: " << dev->getName() << "\n";
        IF_DEBUG(
        std::cout << "    buffer_size=" << dev->getCurrentBufferSizeSamples()
                  << " (requested=" << config::DEV_BLOCK_SIZE << ")\n"
                  << "    sample_rate=" << dev->getCurrentSampleRate() << "\n"
                  << "Compare (generator):" << "\n"
                  << "    buffer_size=" << config::GEN_BLOCK_SIZE << "\n";
        );

        config::SAMPLE_RATE = dev->getCurrentSampleRate();
        config::DEV_BLOCK_SIZE = dev->getCurrentBufferSizeSamples();
    } else {
        std::cout << "No audio device loaded.\n";
        return false;
    }

    if (config::BUFFER_RING_SIZE * config::GEN_BLOCK_SIZE < config::DEV_BLOCK_SIZE) {
        std::cout << "\nWARNING: Underruns likely!\n"
                  << "    Total ring throughput is less than the audio device's buffer size. \n"
                  << "    Increase BUFFER_RING_SIZE or GEN_BLOCK_SIZE, or decrease the requested DEV_BLOCK_SIZE.\n"
                  << "    BUFFER_RING_SIZE=" << config::BUFFER_RING_SIZE
                  << ", GEN_BLOCK_SIZE=" << config::GEN_BLOCK_SIZE
                  << ", DEV_BLOCK_SIZE(actual)=" << config::DEV_BLOCK_SIZE
                  << "; " << config::BUFFER_RING_SIZE
                  << " * " << config::GEN_BLOCK_SIZE
                  << " < " << config::DEV_BLOCK_SIZE << "\n";
        // fail on release builds (likely severe underruns)
        IF_RELEASE(return false;);
    }
    return true;
};