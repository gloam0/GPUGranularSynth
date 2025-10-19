#ifndef GPUGRANULARSYNTH_UTILS_HPP
#define GPUGRANULARSYNTH_UTILS_HPP

#include <chrono>

using namespace std::chrono_literals;

// e.g., 'sleep(10ms)'
template<typename Rep, typename Period>
void sleep(const std::chrono::duration<Rep, Period>& duration) {
    std::this_thread::sleep_for(duration);
}

inline int64_t now_steady_ns() noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

#endif //GPUGRANULARSYNTH_UTILS_HPP