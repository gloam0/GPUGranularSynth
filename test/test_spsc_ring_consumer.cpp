#include <doctest.h>
#include <thread>
#include <dataflow/spsc_ring_consumer.hpp>
#include <config.hpp>



// struct alignas(64) AudioBlock {
//     uint64_t seq = 0;
//     uint32_t samples = 0;
//     uint32_t channels = config::NUM_CHANNELS;
//     // interleaved
//     std::array<float, config::GEN_BLOCK_SIZE * config::NUM_CHANNELS> data{};
// };


TEST_CASE("SPSCRingConsumer: config defaults, P/C, no underruns") {
    int dev_N = 100;  // how many device callbacks you want to simulate
    float gen_dev_ratio = static_cast<float>(config::DEV_BLOCK_SIZE) /
                          static_cast<float>(config::GEN_BLOCK_SIZE);
    int prod_N = static_cast<int>(dev_N * gen_dev_ratio) + 1;
    std::vector<AudioBlock> ring(config::BUFFER_RING_SIZE);

    std::vector<float*> _out(config::NUM_CHANNELS);
    for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
        _out[i] = new float[config::DEV_BLOCK_SIZE];
    }
    float* const* out = _out.data();

    SPSCRingState rs(config::BUFFER_RING_SIZE);
    SPSCRingConsumer rc(rs, ring);

    std::thread prod([&] {
        int ct = 0;
        for (int i = 0; i <= prod_N; i++) {
            uint64_t w_curr{};
            size_t slot{};
            while (!rs.acquire_write(w_curr, slot)) {}
            auto& block = ring[slot];
            block.samples = config::GEN_BLOCK_SIZE;
            block.channels = config::NUM_CHANNELS;
            float* slot_buf = block.data.data();
            for (size_t i = 0; i < config::GEN_BLOCK_SIZE; ++i) {
                for (size_t j = 0; j < config::NUM_CHANNELS; ++j) {
                    slot_buf[config::NUM_CHANNELS * i + j] = ct++;
                }
            }
            block.seq = w_curr;
            rs.publish_write(w_curr);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::thread cons([&] {
        int expected = 0;
        while (expected <= prod_N * config::GEN_BLOCK_SIZE * config::NUM_CHANNELS) {
            rc.process(out, config::NUM_CHANNELS, config::DEV_BLOCK_SIZE);

            for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                    CHECK(out[i][j] == expected++);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    });

    rc.stop();
    prod.join();
    cons.join();
}

TEST_CASE("SPSCRingConsumer: config defaults, P/C, underruns") {
    int dev_N = 100;  // how many device callbacks you want to simulate
    float gen_dev_ratio = static_cast<float>(config::DEV_BLOCK_SIZE) /
                          static_cast<float>(config::GEN_BLOCK_SIZE);
    int prod_N = static_cast<int>(dev_N * gen_dev_ratio) + 1;
    // int dev_N = 2;
    // int gen_dev_factor = config::DEV_BLOCK_SIZE / config::GEN_BLOCK_SIZE + 1;
    // int prod_N = 2 * gen_dev_factor;
    std::vector<AudioBlock> ring(config::BUFFER_RING_SIZE);

    std::vector<float*> _out(config::NUM_CHANNELS);
    for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
        _out[i] = new float[config::DEV_BLOCK_SIZE];
    }
    float* const* out = _out.data();

    SPSCRingState rs(config::BUFFER_RING_SIZE);
    SPSCRingConsumer rc(rs, ring);

    std::thread prod([&] {
        int ct = 1;
        for (int i = 0; i < prod_N; i++) {
            uint64_t w_curr{};
            size_t slot{};
            while (!rs.acquire_write(w_curr, slot)) {}
            auto& block = ring[slot];
            block.samples = config::GEN_BLOCK_SIZE;
            block.channels = config::NUM_CHANNELS;
            float* slot_buf = block.data.data();
            for (size_t i = 0; i < config::GEN_BLOCK_SIZE; ++i) {
                for (size_t j = 0; j < config::NUM_CHANNELS; ++j) {
                    slot_buf[config::NUM_CHANNELS * i + j] = ct++;
                }
            }
            block.seq = w_curr;
            rs.publish_write(w_curr);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread cons([&] {
        int ct = 0;
        int expected = 1;
        while (expected <= prod_N * config::GEN_BLOCK_SIZE * config::NUM_CHANNELS) {
            for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                    out[i][j] = -1.f;
                }
            }
            rc.process(out, config::NUM_CHANNELS, config::DEV_BLOCK_SIZE);
            bool saw_good = false;
            for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                    if (out[i][j] == expected) {
                        saw_good = true;
                        expected++;
                    }
                    CHECK(out[i][j] > 0);
                }
            }
            if (saw_good) ct++;
        }
    });

    rc.stop();
    prod.join();
    cons.join();
}

TEST_CASE("SPSCRingConsumer: dev buffer > gen buffer, P/C, underruns") {
    config::DEV_BLOCK_SIZE = config::GEN_BLOCK_SIZE * 2;
    int dev_N = 100;
    float gen_dev_ratio = static_cast<float>(config::DEV_BLOCK_SIZE) /
                          static_cast<float>(config::GEN_BLOCK_SIZE);
    int prod_N = static_cast<int>(dev_N * gen_dev_ratio) + 1;
    std::vector<AudioBlock> ring(config::BUFFER_RING_SIZE);

    std::vector<float*> _out(config::NUM_CHANNELS);
    for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
        _out[i] = new float[config::DEV_BLOCK_SIZE];
    }
    float* const* out = _out.data();

    SPSCRingState rs(config::BUFFER_RING_SIZE);
    SPSCRingConsumer rc(rs, ring);

    std::thread prod([&] {
        int ct = 1;
        for (int i = 0; i < prod_N; i++) {
            uint64_t w_curr{};
            size_t slot{};
            while (!rs.acquire_write(w_curr, slot)) {}
            auto& block = ring[slot];
            block.samples = config::GEN_BLOCK_SIZE;
            block.channels = config::NUM_CHANNELS;
            float* slot_buf = block.data.data();
            for (size_t i = 0; i < config::GEN_BLOCK_SIZE; ++i) {
                for (size_t j = 0; j < config::NUM_CHANNELS; ++j) {
                    slot_buf[config::NUM_CHANNELS * i + j] = ct++;
                }
            }
            block.seq = w_curr;
            rs.publish_write(w_curr);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread cons([&] {
        int ct = 0;
        int expected = 1;
        while (expected <= prod_N * config::GEN_BLOCK_SIZE * config::NUM_CHANNELS) {
            for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                    out[i][j] = -1.f;
                }
            }
            rc.process(out, config::NUM_CHANNELS, config::DEV_BLOCK_SIZE);
            bool saw_good = false;
            for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                    if (out[i][j] == expected) {
                        saw_good = true;
                        expected++;
                    }
                    CHECK(out[i][j] > 0);
                }
            }
            if (saw_good) ct++;
        }
    });

    rc.stop();
    prod.join();
    cons.join();
}

TEST_CASE("SPSCRingConsumer: dev buffer == gen buffer, P/C, underruns") {
    config::DEV_BLOCK_SIZE = config::GEN_BLOCK_SIZE;
    int dev_N = 100;
    float gen_dev_ratio = static_cast<float>(config::DEV_BLOCK_SIZE) /
                          static_cast<float>(config::GEN_BLOCK_SIZE);
    int prod_N = static_cast<int>(dev_N * gen_dev_ratio) + 1;
    std::vector<AudioBlock> ring(config::BUFFER_RING_SIZE);

    std::vector<float*> _out(config::NUM_CHANNELS);
    for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
        _out[i] = new float[config::DEV_BLOCK_SIZE];
    }
    float* const* out = _out.data();

    SPSCRingState rs(config::BUFFER_RING_SIZE);
    SPSCRingConsumer rc(rs, ring);

    std::thread prod([&] {
        int ct = 1;
        for (int i = 0; i < prod_N; i++) {
            uint64_t w_curr{};
            size_t slot{};
            while (!rs.acquire_write(w_curr, slot)) {}
            auto& block = ring[slot];
            block.samples = config::GEN_BLOCK_SIZE;
            block.channels = config::NUM_CHANNELS;
            float* slot_buf = block.data.data();
            for (size_t i = 0; i < config::GEN_BLOCK_SIZE; ++i) {
                for (size_t j = 0; j < config::NUM_CHANNELS; ++j) {
                    slot_buf[config::NUM_CHANNELS * i + j] = ct++;
                }
            }
            block.seq = w_curr;
            rs.publish_write(w_curr);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread cons([&] {
        int ct = 0;
        int expected = 1;
        while (expected <= prod_N * config::GEN_BLOCK_SIZE * config::NUM_CHANNELS) {
            for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                    out[i][j] = -1.f;
                }
            }
            rc.process(out, config::NUM_CHANNELS, config::DEV_BLOCK_SIZE);
            bool saw_good = false;
            for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                    if (out[i][j] == expected) {
                        saw_good = true;
                        expected++;
                    }
                    CHECK(out[i][j] > 0);
                }
            }
            if (saw_good) ct++;
        }
    });

    rc.stop();
    prod.join();
    cons.join();
}

TEST_CASE("SPSCRingConsumer: dev buffer < gen buffer, P/C, underruns") {
    config::DEV_BLOCK_SIZE = config::GEN_BLOCK_SIZE / 2;
    int dev_N = 100;
    float gen_dev_ratio = static_cast<float>(config::DEV_BLOCK_SIZE) /
                          static_cast<float>(config::GEN_BLOCK_SIZE);
    int prod_N = static_cast<int>(dev_N * gen_dev_ratio) + 1;
    std::vector<AudioBlock> ring(config::BUFFER_RING_SIZE);

    std::vector<float*> _out(config::NUM_CHANNELS);
    for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
        _out[i] = new float[config::DEV_BLOCK_SIZE];
    }
    float* const* out = _out.data();

    SPSCRingState rs(config::BUFFER_RING_SIZE);
    SPSCRingConsumer rc(rs, ring);

    std::thread prod([&] {
        int ct = 1;
        for (int i = 0; i < prod_N; i++) {
            uint64_t w_curr{};
            size_t slot{};
            while (!rs.acquire_write(w_curr, slot)) {}
            auto& block = ring[slot];
            block.samples = config::GEN_BLOCK_SIZE;
            block.channels = config::NUM_CHANNELS;
            float* slot_buf = block.data.data();
            for (size_t i = 0; i < config::GEN_BLOCK_SIZE; ++i) {
                for (size_t j = 0; j < config::NUM_CHANNELS; ++j) {
                    slot_buf[config::NUM_CHANNELS * i + j] = ct++;
                }
            }
            block.seq = w_curr;
            rs.publish_write(w_curr);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread cons([&] {
        int ct = 0;
        int expected = 1;
        while (expected <= prod_N * config::GEN_BLOCK_SIZE * config::NUM_CHANNELS) {
            for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                    out[i][j] = -1.f;
                }
            }
            rc.process(out, config::NUM_CHANNELS, config::DEV_BLOCK_SIZE);
            bool saw_good = false;
            for (size_t j = 0; j < config::DEV_BLOCK_SIZE; ++j) {
                for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
                    if (out[i][j] == expected) {
                        saw_good = true;
                        expected++;
                    }
                    CHECK(out[i][j] > 0);
                }
            }
            if (saw_good) ct++;
        }
    });

    rc.stop();
    prod.join();
    cons.join();
}