add_subdirectory(utils)
add_subdirectory(dataflow)
add_subdirectory(audio)

add_executable(GPUGranularSynth
        main.cpp
)

target_include_directories(GPUGranularSynth
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(GPUGranularSynth
        PUBLIC
        juce::juce_recommended_warning_flags
        juce::juce_recommended_config_flags
        $<$<CONFIG:Release>:juce::juce_recommended_lto_flags>
        PRIVATE
        juce::juce_core
        juce::juce_events
        juce::juce_audio_devices
        CUDA::cudart
        utils
        dataflow
        audio
)