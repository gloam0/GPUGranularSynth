add_library(controls
        control_poller.cpp
)

target_include_directories(controls
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(controls
        PRIVATE
        # CUDA::cudart
        # juce::juce_core
        # juce::juce_events
        juce::juce_audio_devices
)