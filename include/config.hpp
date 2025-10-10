#ifndef GPUGRANULARSYNTH_CONFIG_H
#define GPUGRANULARSYNTH_CONFIG_H

#if PROJ_DEBUG
    #define IF_DEBUG(expr) do { expr; } while (0)
#else
    #define IF_DEBUG(expr) ((void)0)
#endif

#endif //GPUGRANULARSYNTH_CONFIG_H