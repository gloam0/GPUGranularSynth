add_executable(unit_tests
        main_tests.cpp
        test_spsc_ring_state.cpp
        test_spsc_ring_consumer.cpp
)

target_include_directories(unit_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external/doctest/doctest/
)

target_link_libraries(unit_tests
        PRIVATE
        utils
        dataflow
        audio
        juce::juce_core
        juce::juce_events
        juce::juce_audio_devices
        CUDA::cudart
)

target_compile_features(unit_tests PRIVATE cxx_std_20)

enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)

add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS unit_tests
)
