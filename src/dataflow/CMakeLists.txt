add_library(dataflow
        spsc_ring_state.cpp
)

target_include_directories(dataflow
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(dataflow
        PRIVATE
        CUDA::cudart
)