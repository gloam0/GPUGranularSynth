#ifndef GPUGRANULARSYNTH_CUDA_DEV_INFO_HPP
#define GPUGRANULARSYNTH_CUDA_DEV_INFO_HPP

#include <cuda_runtime.h>

int cuda_get_device_count();
cudaDeviceProp cuda_get_device_properties(int dev);
void cuda_print_device_info(int dev_id, const cudaDeviceProp* p);
void cuda_print_device_info_all();

#endif //GPUGRANULARSYNTH_CUDA_DEV_INFO_HPP