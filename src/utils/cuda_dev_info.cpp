#include <utils/utils.hpp>
#include <utils/cuda_dev_info.hpp>

int cuda_get_device_count() {
    int deviceCount = 0;
    CUDA_CHECK(cudaGetDeviceCount(&deviceCount));
    return deviceCount;
}

cudaDeviceProp cuda_get_device_properties(const int dev) {
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, dev));
    return prop;
}

void cuda_print_device_info(const cudaDeviceProp* p) {
    std::cout << "  name:                   " << p->name << "\n"
              << "  compute maj.min:        " << p->major << "." << p->minor << "\n"
              << "  # SMs:                  " << p->multiProcessorCount << "\n"
              << "  max threads per SM:     " << p->maxThreadsPerMultiProcessor << "\n"
              << "  max resident threads:   " << p->multiProcessorCount * p->maxThreadsPerMultiProcessor << "\n"
              << "  max threads per block:  " << p->maxThreadsPerBlock << "\n"
              << "  max grid size:          " << p->maxGridSize[0] << ", "
                  << p->maxGridSize[1] << ", " << p->maxGridSize[2] << ")\n"
              << "  total global mem:       " << p->totalGlobalMem / (1024 * 1024) << " MB\n"
              << "  shared mem per block:   " << p->sharedMemPerBlock / 1024 << " KB\n"
              << "  warp size:              " << p->warpSize << "\n";
}

void cuda_print_device_info_all() {
    int deviceCount = cuda_get_device_count();
    for (int dev = 0; dev < deviceCount; ++dev) {
        std::cout << "Device " << dev << "\n";
        cudaDeviceProp prop = cuda_get_device_properties(dev);
        cuda_print_device_info(&prop);
    }
}
