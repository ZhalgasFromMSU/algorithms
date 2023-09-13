#include <algo/sync/queue.hpp>

#include <gtest/gtest.h>

#include <thread>

using namespace algo;

TEST(Queue, Simple) {
    LfQueue<int> q {4};
    for (int i : {1, 2, 3, 4}) {
        ASSERT_TRUE(q.Push(std::move(i)));
    }
    ASSERT_FALSE(q.Push(5));

    for (int i : {1, 2, 3, 4}) {
        ASSERT_EQ(q.Pop().value(), i);
    }
    ASSERT_FALSE(q.Pop());
}

TEST(Queue, BufferReuse) {
    LfQueue<int> q {1};
    for (int i : {1, 2}) {
        ASSERT_TRUE(q.Push(std::move(i)));
        ASSERT_EQ(q.Pop().value(), i);
    }
}

TEST(Queue, Datarace) {
    LfQueue<size_t> q {100};

    std::atomic<size_t> counter = 0;
    const size_t max_count = 10'000;
    std::atomic<size_t> num_read = 0;
    std::atomic<size_t> total_sum = 0;

    {
        size_t num_threads = 20;
        std::vector<std::jthread> producers(num_threads);
        std::vector<std::jthread> consumers(num_threads);

        for (auto& producer : producers) {
            producer = std::jthread([&] {
                while (true) {
                    size_t num = counter.fetch_add(1);
                    if (num > max_count) {
                        break;
                    }

                    while (!q.Push(std::move(num))) {
                    }
                }
            });
        }

        for (auto& consumer : consumers) {
            consumer = std::jthread([&] {
                while (true) {
                    auto popped = q.Pop();
                    while (!popped && num_read <= max_count) {
                        popped = q.Pop();
                    }

                    if (popped) {
                        num_read += 1;
                        total_sum += popped.value();
                    } else {
                        break;
                    }
                }
            });
        }
    }

    ASSERT_EQ(total_sum, max_count * (max_count + 1) / 2);
}

TEST(Queue, Ordering) {
    LfQueue<int> q {1};

    size_t num_threads = 2;
    std::vector<std::thread> prod(num_threads), cons(num_threads);

    std::atomic_flag stopped;

    for (size_t i = 0; i < num_threads; ++i) {
        prod[i] = std::thread{[&] {
            while (!stopped.test()) {
                q.Push(1);
            }
        }};

        cons[i] = std::thread{[&] {
            while (!stopped.test()) {
                ASSERT_EQ(q.Pop().value_or(1), 1);
            }
        }};
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stopped.test_and_set();
    for (size_t i = 0; i < num_threads; ++i) {
        if (prod[i].joinable()) {
            prod[i].join();
        }

        if (cons[i].joinable()) {
            cons[i].join();
        }
    }
}
