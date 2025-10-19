#include <doctest.h>
#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

template<typename Rep, typename Period>
void sleep(const std::chrono::duration<Rep, Period>& duration) {
    std::this_thread::sleep_for(duration);
}

static std::atomic<int64_t> mock_now_ns{0};
int64_t now_steady_ns() noexcept { return mock_now_ns.load(std::memory_order_acquire); }

void set_now_ns(int64_t ns) { mock_now_ns.store(ns, std::memory_order_release); }
void advance_ns(int64_t ns) { mock_now_ns.fetch_add(ns, std::memory_order_acq_rel); }

inline std::atomic<uint64_t> dev_sample_count{0};

inline uint64_t get_dev_sample_count() noexcept {
    return dev_sample_count.load(std::memory_order_relaxed);
}

struct SampleClock {
    explicit SampleClock(const double sr) : sr(sr) {};

    std::atomic<double> sr{48000.0};
    std::atomic<uint64_t> sample_count{0};
    std::atomic<int64_t> time_ns{0};

    uint64_t est_now_samples() const noexcept {
        const auto t0 = time_ns.load(std::memory_order_acquire);
        const auto s0 = sample_count.load(std::memory_order_acquire);
        const double rate = sr.load(std::memory_order_acquire);
        const int64_t dt = now_steady_ns() - t0;
        const double inc = dt > 0 ? static_cast<double>(dt) * rate / 1e9 : 0.;
        return s0 + static_cast<uint64_t>(inc);
    }

    void update() {
        sample_count.store(get_dev_sample_count(), std::memory_order_release);
        time_ns.store(now_steady_ns(), std::memory_order_release);
    }
};

void reset() {
    dev_sample_count.store(0, std::memory_order_release);
    set_now_ns(0);
}

TEST_CASE("SampleClock: update()") {
    reset();
    SampleClock clk{48000.0};

    dev_sample_count.store(1024, std::memory_order_release);
    set_now_ns(1234);

    clk.update();

    CHECK(clk.sample_count.load(std::memory_order_acquire) == 1024);
    CHECK(clk.time_ns.load(std::memory_order_acquire) == 1234);
}

TEST_CASE("SampleClocK: est_now_samples()") {
    reset();
    SampleClock clk{48000.0};

    dev_sample_count.store(12345, std::memory_order_release);
    set_now_ns(2'000'000'000); // t0 = 2.0 s
    clk.update();

    // 0.5s, +24000 (48khz)
    advance_ns(500000000);
    CHECK(clk.est_now_samples() == 12345 + 24'000);

    // 0.5s, +24000 (48khz)
    advance_ns(500000000);
    CHECK(clk.est_now_samples() == 12345 + 48'000);
}

TEST_CASE("SampleClock: est_now_samples() 2") {
    reset();
    SampleClock clk{48000.0};

    // blocks of 256 samples, 5.333 ms per block
    dev_sample_count.store(0, std::memory_order_release);
    set_now_ns(0);
    clk.update();

    advance_ns(2'666'666);
    CHECK(clk.est_now_samples() == 127);
    advance_ns(1);
    CHECK(clk.est_now_samples() == 128);

    dev_sample_count.store(256, std::memory_order_release);
    advance_ns(2'666'666);
    clk.update();
    CHECK(clk.sample_count.load(std::memory_order_acquire) == 256);

    // 0.5ms = 24 samples
    advance_ns(500'000);
    CHECK(clk.est_now_samples() == 256 + 24);
}

TEST_CASE("SampleClock: est_now_samples() is monotonic") {
    reset();
    SampleClock clk{48000.0};

    dev_sample_count.store(777, std::memory_order_release);
    set_now_ns(123456);
    clk.update();

    // time goes backwards
    set_now_ns(100000);
    CHECK(clk.est_now_samples() == 777);

    set_now_ns(123456);
    CHECK(clk.est_now_samples() == 777);
}

TEST_CASE("SampleClock: est_now_samples() fractional increments floor'd") {
    reset();
    SampleClock clk{48000.0};

    dev_sample_count.store(500, std::memory_order_release);
    set_now_ns(1000000); // 0.1 s
    clk.update();

    // 10ns: 48k * 10e-9 = 0.00048 samples
    advance_ns(10);
    CHECK(clk.est_now_samples() == 500);

    // 21000ns: 48k * 21000e-9 = 1.008 samples
    advance_ns(21000);
    CHECK(clk.est_now_samples() == 501);
}