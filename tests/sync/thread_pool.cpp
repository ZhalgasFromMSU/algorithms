#include <algo/sync/thread_pool.hpp>

#include <gtest/gtest.h>

#include <functional>

struct ThreadPool : testing::Test {
  std::atomic<std::size_t> counter = 0;
};

struct Task {
  void operator()() {
    *counter += 1;
  }

  std::atomic<std::size_t>* counter;
};

TEST_F(ThreadPool, SingleProducer) {
  algo::ThreadPool<Task> pool(8, 100);
  pool.Start();

  std::size_t max = 100'000;

  for (std::size_t i = 0; i < max; ++i) {
    ASSERT_TRUE(pool.Enqueue(Task{.counter = &counter}));
  }
  pool.Stop();
  ASSERT_FALSE(pool.Enqueue(Task{.counter = &counter}));
  ASSERT_EQ(counter, max);
}

TEST_F(ThreadPool, MultipleProducers) {
  algo::ThreadPool<Task> pool(4, 1'000);
  pool.Start();

  std::size_t max = 100'000;
  std::size_t prods_count = 4;
  std::vector<std::jthread> prods(prods_count);
  std::atomic<std::size_t> local_counter = 0;
  std::latch prods_latch(prods_count);

  for (auto& thread : prods) {
    thread = std::jthread{[&] {
      while (local_counter.fetch_add(1) < max) {
        ASSERT_TRUE(pool.Enqueue(Task{.counter = &counter}));
      }
      prods_latch.count_down();
    }};
  }
  prods_latch.wait();
  pool.Stop();
  ASSERT_EQ(counter, max);
}
