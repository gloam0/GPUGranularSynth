#ifndef GPUGRANULARSYNTH_CONFIG_H
#define GPUGRANULARSYNTH_CONFIG_H

#if PROJ_DEBUG
    #define IF_DEBUG(expr) do { expr; } while (0)
#else
    #define IF_DEBUG(expr) ((void)0)
#endif


namespace config {

    constexpr int CUDA_SELECTED_DEVICE = 0;

}

#endif //GPUGRANULARSYNTH_CONFIG_H