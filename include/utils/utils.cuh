#ifndef GPUGRANULARSYNTH_UTILS_CUH
#define GPUGRANULARSYNTH_UTILS_CUH

#include <chrono>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include <cuda_runtime.h>

#define CUDA_CHECK(x) cuda_check((x), #x, __FILE__, __LINE__)

inline void cuda_check(cudaError_t err, const char *func, const char *file, int line) {
    if (err != cudaSuccess) {
        std::cerr << "[CUDA_CHECK] " << cudaGetErrorName(err)
            << "\n" << cudaGetErrorString(err) << "\n"
            << "in:   " << file << ":" << line << "\n"
            << "call: " << func << "\n";
        std::exit(EXIT_FAILURE);
    }
}

#define CUDA_CHECK_LAST() cuda_check_last(__FILE__, __LINE__)

inline void cuda_check_last(const char* file, int line) {
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "[CUDA_CHECK_LAST] " << cudaGetErrorName(err)
            << "\n" << cudaGetErrorString(err) << "\n"
            << "after kernel launch at: " << file << ":" << line << "\n";
        std::exit(EXIT_FAILURE);
    }
}

#endif //GPUGRANULARSYNTH_UTILS_CUH