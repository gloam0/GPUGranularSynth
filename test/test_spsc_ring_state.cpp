#include <doctest.h>
#include <dataflow/spsc_ring_state.hpp>
#include <thread>
#include <vector>
#include <atomic>

TEST_CASE("SPSCRingState: Empty ring - acquire_read() fails, underruns = 1, get_last_good() fails") {
    SPSCRingState rs(3);
    uint64_t r_curr{};
    size_t slot{};

    CHECK_FALSE(rs.acquire_read(r_curr, slot));
    CHECK(rs.get_underruns() == 1);
    CHECK_FALSE(rs.get_last_good(slot));
    CHECK(rs.get_read_count() == 0);
    CHECK(rs.get_write_count() == 0);
}

TEST_CASE("SPSCRingState: One write, one read") {
    SPSCRingState rs(3);
    uint64_t w_curr{};
    uint64_t r_curr{};
    size_t slot{};

    REQUIRE(rs.acquire_write(w_curr, slot));
    CHECK(w_curr == 0);
    CHECK(slot == 0);
    rs.publish_write(w_curr);

    REQUIRE(rs.acquire_read(r_curr, slot));
    CHECK(r_curr == 0);
    CHECK(slot == 0);
    rs.publish_read(r_curr);

    CHECK(rs.get_underruns() == 0);
    CHECK(rs.get_read_count() == 1);
    CHECK(rs.get_write_count() == 1);
}

TEST_CASE("SPSCRingState: Writes blocked if 'full'") {
    SPSCRingState rs(3);
    uint64_t w_curr{};
    size_t slot{};

    for (int i = 0; i < 3; i++) {
        REQUIRE(rs.acquire_write(w_curr, slot));
        rs.publish_write(w_curr);
    }

    CHECK_FALSE(rs.acquire_write(w_curr, slot));

    CHECK(rs.get_underruns() == 0);
    CHECK(rs.get_read_count() == 0);
    CHECK(rs.get_write_count() == 3);
}

TEST_CASE("SPSCRingState: If empty, get_last_good returns most recent consumed slot") {
    SPSCRingState rs(3);
    uint64_t w_curr{};
    size_t slot{};
    uint64_t r_curr{};

    REQUIRE(rs.acquire_write(w_curr, slot));
    CHECK(slot == 0);
    rs.publish_write(w_curr);
    REQUIRE(rs.acquire_write(w_curr, slot));
    CHECK(slot == 1);
    rs.publish_write(w_curr);
    REQUIRE(rs.acquire_read(r_curr, slot));
    CHECK(slot == 0);
    rs.publish_read(r_curr);
    REQUIRE(rs.acquire_read(r_curr, slot));
    CHECK(slot == 1);
    rs.publish_read(r_curr);

    CHECK_FALSE(rs.acquire_read(r_curr, slot));

    size_t last{};
    REQUIRE(rs.get_last_good(last));
    CHECK(last == 1);
}

TEST_CASE("SPSCRingState: Simple producer/consumer threads") {
    SPSCRingState rs(3);
    constexpr int N = 100;
    std::vector<int> buf(rs.slots);

    std::thread prod([&] {
        for (int i = 0; i < N; i++) {
            uint64_t w_curr{};
            size_t slot{};
            while (!rs.acquire_write(w_curr, slot)) {}
            buf[slot] = i;
            rs.publish_write(w_curr);
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    });

    std::thread cons([&] {
        int expected = 0;
        size_t last = 0;
        while (expected < N) {
            uint64_t r_curr{};
            size_t slot{};

            if (!rs.acquire_read(r_curr, slot)) {
                rs.get_last_good(slot);
                CHECK(slot == last);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            CHECK(buf[slot] == expected);
            rs.publish_read(r_curr);
            last = slot;
            ++expected;
        }

        uint64_t r_curr{};
        size_t slot{};

        CHECK_FALSE(rs.acquire_read(r_curr, slot));
        rs.get_last_good(slot);
        CHECK(slot == (N - 1) % rs.slots);
    });
    prod.join();
    cons.join();

    CHECK(rs.get_write_count() == N);
    CHECK(rs.get_read_count() == N);
    CHECK(rs.get_underruns() > 0);
}
