#include <iostream>

#include <config.hpp>
#include <utils/cuda_dev_info.hpp>

int main() {
    int dev_count = cuda_get_device_count();
    int dev_use = (dev_count > 1) ? config::CUDA_SELECTED_DEVICE : 0;
    cudaSetDevice(dev_use);
    std::cout << "Selected device " << dev_use << "\n";
    IF_DEBUG(cuda_print_device_info_all());

    return 0;
}