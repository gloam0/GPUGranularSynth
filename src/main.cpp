#include <iostream>

#include <config.hpp>
#include <utils/cuda_dev_info.hpp>

int main() {
    IF_DEBUG(cuda_print_device_info_all());
    return 0;
}