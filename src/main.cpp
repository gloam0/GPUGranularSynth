#include <array>
#include <iostream>
#include <chrono>
#include <config.hpp>
#include <utils/cuda_dev_info.hpp>
#include <utils/utils.hpp>

#include <audio/audio_juce.hpp>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

int run_vecadd_cuda();
int main() {
    return run_vecadd_cuda();
}