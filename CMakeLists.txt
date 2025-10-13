cmake_minimum_required(VERSION 3.22)

if(NOT CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES native)
endif()

project(GPUGranularSynth LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

find_package(CUDAToolkit REQUIRED)

# JUCE {
set(JUCE_MODULES_ONLY ON CACHE BOOL "" FORCE)

add_compile_definitions(
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

if(WIN32)
    add_compile_definitions(
            JUCE_WASAPI=1 JUCE_DIRECTSOUND=1
            #$<$<CONFIG:Debug>:JUCE_ASIO=1>
            $<$<CONFIG:Release>:JUCE_ASIO=0>
    )
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(JUCE_ALSA=1 JUCE_JACK=1)
endif()

add_subdirectory(external/JUCE)
# } JUCE

add_compile_definitions(
        $<$<CONFIG:Debug>:PROJ_DEBUG=1>
        $<$<CONFIG:Release>:PROJ_DEBUG=0>
)

add_subdirectory(src)

add_subdirectory(test)
