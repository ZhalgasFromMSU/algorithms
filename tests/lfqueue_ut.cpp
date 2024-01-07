#include <algorithm>
#include <thread>

#include <gtest/gtest.h>

import algo;

TEST(LfQueue, Trivial) {
  algo::LfQueue<int> queue(1, 1, 1);
  ASSERT_TRUE(queue.TryPush(1));
  ASSERT_FALSE(queue.TryPush(2));
  ASSERT_TRUE(queue.TryPop().value() == 1);
  ASSERT_FALSE(queue.TryPop());
}

TEST(LfQueue, Pop) {
  algo::LfQueue<int> queue(1, 1, 1);
  std::jthread consumer{[&] { ASSERT_EQ(queue.Pop(), 1); }};
  std::jthread producer{[&] { ASSERT_TRUE(queue.TryPush(1)); }};
}

TEST(LfQueue, SpSc) {
  algo::LfQueue<std::size_t> queue(1000, 1, 1);

  std::jthread consumer{[&] {
    std::size_t counter = 0;
    while (counter < 1000) {
      ASSERT_EQ(queue.Pop(), counter++);
    }
    ASSERT_FALSE(queue.TryPop());
  }};

  std::jthread producer{[&] {
    std::size_t counter = 0;
    while (counter < 1000) {
      ASSERT_TRUE(queue.TryPush(counter++));
    }
  }};
}

TEST(LfQueue, SpMc) {
  std::size_t n_consumers = 8;
  algo::LfQueue<std::size_t> queue(8, 1, n_consumers);

  std::vector<std::jthread> consumers;
  for (std::size_t i = 0; i < n_consumers; ++i) {
    consumers.emplace_back([&, i = i] { ASSERT_EQ(queue.Pop(), i); });
  }

  std::jthread producer{[&] {
    for (std::size_t i = 0; i < n_consumers; ++i) {
      ASSERT_TRUE(queue.TryPush(std::move(i)));
    }
  }};
}

TEST(LfQueue, MpMc) {
  std::size_t n_consumers = 8;
  std::size_t n_producers = 8;
  algo::LfQueue<std::size_t> queue(1000, n_producers, n_consumers);

  std::vector<std::atomic_flag> flags(1000);
  {
    std::atomic<std::size_t> consumer_counter = 0;
    std::vector<std::jthread> consumers(n_consumers);
    for (auto&& consumer : consumers) {
      consumer = std::jthread{[&] {
        while (consumer_counter.fetch_add(1, std::memory_order_relaxed) <
               1000) {
          flags[queue.Pop()].test_and_set(std::memory_order_relaxed);
        }
      }};
    }

    std::atomic<std::size_t> producer_counter = 0;
    std::vector<std::jthread> producers(n_producers);
    for (auto&& producer : producers) {
      producer = std::jthread{[&] {
        while (true) {
          std::size_t to_push =
              producer_counter.fetch_add(1, std::memory_order_relaxed);
          if (to_push >= 1000) {
            break;
          }

          queue.Push(std::move(to_push));
        }
      }};
    }
  }

  for (auto&& flag : flags) {
    ASSERT_TRUE(
        std::ranges::all_of(flags, [](auto& flag) { return flag.test(); }));
  }
}
