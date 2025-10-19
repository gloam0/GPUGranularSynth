add_library(audio
        audio_juce.cpp
)

target_include_directories(audio
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(audio
        PRIVATE
        CUDA::cudart
        juce::juce_core
        # juce::juce_events
        juce::juce_audio_devices
)