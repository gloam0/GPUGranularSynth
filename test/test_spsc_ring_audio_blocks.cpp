#include <doctest.h>
#include <thread>
#include <future>
#include <dataflow/spsc_ring_audio_blocks.hpp>
#include <config.hpp>
#include <utils/utils.hpp>


TEST_CASE("SPSCRingAudioBlocks: config defaults, P/C, no underruns") {
    std::vector<AudioBlock> ring(config::BUFFER_RING_SIZE);
    std::vector<std::vector<float>> out_storage(config::NUM_CHANNELS,
        std::vector<float>(config::DEV_BLOCK_SIZE, 0.f));
    std::vector<float*> out_ptrs(config::NUM_CHANNELS);
    for (size_t i = 0; i < config::NUM_CHANNELS; i++) {
        out_ptrs[i] = out_storage[i].data();
    }
    float* const* out = out_ptrs.data();

    SPSCRingState rs(config::BUFFER_RING_SIZE);
    SPSCRingAudioBlocks rc(rs, ring);

    std::atomic<bool> prod_done = false;
    std::atomic<bool> stop_cons = false;
    std::promise<void> prod_block_done_promise;
    std::future<void> prod_block_done_future = prod_block_done_promise.get_future();

    rc.start();
    // fill ring
    std::thread prod([&] {
        float v = 1.f;
        for (int i = 0; i < config::BUFFER_RING_SIZE; i++) {
            uint64_t w_curr{};
            size_t slot{};
            while (!rs.acquire_write(w_curr, slot)) {}
            ring[slot].samples = config::GEN_BLOCK_SIZE;
            for (uint32_t ch = 0; ch < config::NUM_CHANNELS; ch++) {
                for (uint32_t i = 0; i < config::GEN_BLOCK_SIZE; i++) {
                    ring[slot].ch(ch)[i] = v++;
                }
            }
            rs.publish_write(w_curr);
        }
        prod_block_done_promise.set_value();
        while (!prod_done) {
            std::this_thread::yield();
        }
    });

    prod_block_done_future.wait();
    prod_done = true;
    prod.join();

    std::atomic<bool> seen_values(false);
    // read for a while, consuming ring and inducing underruns
    std::thread cons([&] {
        bool seen = false;
        int frame_idx = 0;
        while (!stop_cons && rs.get_read_count() < rs.get_write_count()) {
            rc.consume(out, config::NUM_CHANNELS, config::DEV_BLOCK_SIZE);

            for (uint32_t ch = 0; ch < config::NUM_CHANNELS; ++ch) {
                for (uint32_t i = 0; i < config::DEV_BLOCK_SIZE; ++i) {
                    if (out[ch][i] == 0.f) {
                        CHECK(seen == false);
                        continue;
                    }
                    if (!seen) {
                        seen = true;
                        seen_values.store(true);
                    }
                    // determine expected value arithmetically
                    int global_i = frame_idx * config::DEV_BLOCK_SIZE + i;
                    int block_i = global_i / config::GEN_BLOCK_SIZE;
                    int in_block_i  = global_i % config::GEN_BLOCK_SIZE;
                    float expected = 1.f
                        + block_i * config::GEN_BLOCK_SIZE * config::NUM_CHANNELS
                        + ch * config::GEN_BLOCK_SIZE
                        + in_block_i;
                    CHECK(out[ch][i] == expected);
                }
            }
            ++frame_idx;
            sleep(1ms);
        }
    });

    sleep(50ms);

    stop_cons = true;

    cons.join();
    rc.stop();

    CHECK(seen_values.load(std::memory_order_acquire));
}
