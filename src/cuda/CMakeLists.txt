add_library(cuda OBJECT
        vecadd.cu
)

target_include_directories(cuda
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
