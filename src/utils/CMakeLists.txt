add_library(utils
        cuda_dev_info.cpp
)

target_include_directories(utils
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(utils
        PRIVATE
        CUDA::cudart
)