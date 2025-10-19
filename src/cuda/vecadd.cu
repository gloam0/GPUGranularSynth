#include <config.hpp>
#include <utils/utils.hpp>
#include <utils/cuda_dev_info.hpp>
#include <cuda_runtime.h>


__global__ void vecAdd(const float* __restrict__ A,
    const float* __restrict__ B,
    float* __restrict__ C,
    int N) {
    for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < N; i += blockDim.x * gridDim.x) {
        C[i] = A[i] + B[i];
    }
}

int run_vecadd_cuda() {
    int dev_count = cuda_get_device_count();
    int dev_use = (dev_count > 1) ? config::CUDA_SELECTED_DEVICE : 0;
    cudaDeviceProp dev_use_prop = cuda_get_device_properties(dev_use);

    CUDA_CHECK(cudaSetDevice(dev_use));
    cuda_print_device_info(dev_use, &dev_use_prop);

    const int N = 1<<27;
    std::cout << "N: " << N << std::endl;
    const size_t bytes = N * sizeof(float);

    float* hA = (float*)malloc(bytes);
    float* hB = (float*)malloc(bytes);
    float* hC = (float*)malloc(bytes);
    for (int i = 0; i < N; i++) { hA[i] = i * 1.f; hB[i] = i * 2.f; }

    float *dA, *dB, *dC;
    CUDA_CHECK(cudaMalloc(&dA, bytes));
    CUDA_CHECK(cudaMalloc(&dB, bytes));
    CUDA_CHECK(cudaMalloc(&dC, bytes));
    CUDA_CHECK(cudaMemcpy(dA, hA, bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dB, hB, bytes, cudaMemcpyHostToDevice));

    int block = 256;
    int blocksPerSM = 0;
    CUDA_CHECK(cudaOccupancyMaxActiveBlocksPerMultiprocessor(&blocksPerSM, vecAdd, block, 0));
    int sms = dev_use_prop.multiProcessorCount;
    int gridDim = sms * (blocksPerSM > 0 ? blocksPerSM : 32);
    std::cout << "sms: " << sms << "  gridDim: " << gridDim << std::endl;

    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));
    vecAdd<<<gridDim, block>>>(dA, dB, dC, N);
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float ms = 0.f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));

    CUDA_CHECK(cudaMemcpy(hC, dC, bytes, cudaMemcpyDeviceToHost));
    for (int i = 0; i < 5; i++) {
        std::cout << i << ": " << hC[i] << std::endl;
    }
    double gb = 3.0 * bytes / 1e9;
    std::cout << "Elapsed: (ms) " << ms << " Throughput: (gb/s) " << gb / (ms/1e3) << std::endl;

    CUDA_CHECK(cudaFree(dA));
    CUDA_CHECK(cudaFree(dB));
    CUDA_CHECK(cudaFree(dC));
    free(hA); free(hB); free(hC);
    return 0;
}