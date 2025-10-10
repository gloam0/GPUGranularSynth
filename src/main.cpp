#include <iostream>
#include <utils/cuda_dev_info.hpp>

int main() {
    cuda_print_device_info_all();
    return 0;
}