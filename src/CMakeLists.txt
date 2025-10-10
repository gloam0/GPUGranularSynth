add_subdirectory(utils)

add_executable(GPUGranularSynth
        main.cpp
)

target_include_directories(GPUGranularSynth
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(GPUGranularSynth
        PRIVATE
        CUDA::cudart
        utils
)