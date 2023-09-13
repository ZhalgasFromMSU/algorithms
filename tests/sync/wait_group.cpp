#include <algo/sync/wait_group.hpp>

#include <gtest/gtest.h>

#include <thread>

using namespace algo;

TEST(WaitGroup, WaitGroup) {
    WaitGroup wg;
    ASSERT_TRUE(wg.Inc());
    ASSERT_TRUE(wg.Dec());
    ASSERT_FALSE(wg.Dec());
    wg.Block();
    ASSERT_FALSE(wg.Inc());
    ASSERT_FALSE(wg.Dec());
    wg.BlockAndWait();
}

TEST(WaitGroup, Threaded) {
    WaitGroup wg;
    std::atomic<size_t> inc_counter = 0;
    size_t dec_counter = 0;

    std::thread inc_thread {[&wg, &inc_counter] {
        while (!wg.Waited()) {
            if (wg.Inc()) {
                inc_counter += 1;
            }
        }
    }};

    std::thread dec_thread {[&wg, &dec_counter] {
        while (!wg.Waited()) {
            if (wg.Dec()) {
                dec_counter += 1;
            }
        }
    }};

    while (inc_counter == 0) {
    }

    wg.BlockAndWait();
    inc_thread.join();
    dec_thread.join();
    ASSERT_TRUE(wg.Blocked() && wg.Waited());
    ASSERT_TRUE(inc_counter > 0 && inc_counter == dec_counter)
        << inc_counter << '\t' << dec_counter;
}
