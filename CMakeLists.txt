cmake_minimum_required(VERSION 3.18)
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES native)
endif()
project(GPUGranularSynth LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

find_package(CUDAToolkit REQUIRED)

add_subdirectory(src)

target_compile_definitions(GPUGranularSynth PRIVATE
        $<$<CONFIG:Debug>:PROJ_DEBUG=1>
        $<$<CONFIG:Release>:PROJ_DEBUG=0>
)