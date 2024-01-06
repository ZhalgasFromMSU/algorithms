#include <gtest/gtest.h>
#include <thread>

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

