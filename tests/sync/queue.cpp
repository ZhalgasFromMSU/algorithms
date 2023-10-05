#include <algo/sync/queue.hpp>

#include <gtest/gtest.h>

#include <thread>

using namespace algo;

TEST(Queue, Simple) {
  LfQueue<int> q{4};
  for (int i : {1, 2, 3, 4}) {
    ASSERT_TRUE(q.TryPush(std::move(i)));
  }
  ASSERT_FALSE(q.TryPush(5));

  for (int i : {1, 2, 3, 4}) {
    ASSERT_EQ(q.TryPop().value(), i);
  }
  ASSERT_FALSE(q.TryPop());
}

TEST(Queue, BufferReuse) {
  LfQueue<int> q{1};
  for (int i : {1, 2}) {
    ASSERT_TRUE(q.TryPush(std::move(i)));
    ASSERT_EQ(q.TryPop().value(), i);
  }
}

TEST(Queue, Spsc) {
  int max = 1'000'000;
  SpscQueue<int> q{1};
  std::jthread prod{[&] {
    for (int i = 0; i < max; ++i) {
      q.Push(std::move(i));
    }
  }};

  std::jthread con{[&] {
    for (int i = 0; i < max; ++i) {
      ASSERT_EQ(q.Pop(), i);
    }
  }};

  prod.join();
  con.join();

  ASSERT_FALSE(q.TryPop());
}

TEST(Queue, Datarace) {
  /*
   * Test that all elements are pushed/popped once and only once
   */
  std::size_t threads = 10;
  LfQueue<std::size_t> q{10};
  constexpr std::size_t max_counter = 1'000'000;
  std::atomic<std::size_t> pushed, popped;
  std::vector<std::atomic_flag> flags(max_counter);

  {
    std::vector<std::jthread> prods(threads);
    std::vector<std::jthread> cons(threads);
    for (auto& prod : prods) {
      prod = std::jthread{[&] {
        while (true) {
          std::size_t cur = pushed.fetch_add(1, std::memory_order_relaxed);
          if (cur >= max_counter) {
            break;
          }

          q.Push(std::move(cur));
        }
      }};
    }

    for (auto& con : cons) {
      con = std::jthread{[&] {
        while (true) {
          std::size_t cur = popped.fetch_add(1, std::memory_order_relaxed);
          if (cur >= max_counter) {
            break;
          }

          std::size_t front = q.Pop();
          ASSERT_EQ(flags.at(front).test_and_set(std::memory_order_relaxed),
                    false);
        }
      }};
    }
  }

  for (const auto& flag : flags) {
    ASSERT_TRUE(flag.test());
  }
}

// TEST(Queue, Ordering) {
// LfQueue<int> q{1};

// size_t num_threads = 2;
// std::vector<std::thread> prod(num_threads), cons(num_threads);

// std::atomic_flag stopped;

// for (size_t i = 0; i < num_threads; ++i) {
// prod[i] = std::thread{[&] {
// while (!stopped.test()) {
// q.TryPush(1);
//}
//}};

// cons[i] = std::thread{[&] {
// while (!stopped.test()) {
// ASSERT_EQ(q.TryPop().value_or(1), 1);
//}
//}};
//}

// std::this_thread::sleep_for(std::chrono::milliseconds(10));
// stopped.test_and_set();
// for (size_t i = 0; i < num_threads; ++i) {
// if (prod[i].joinable()) {
// prod[i].join();
//}

// if (cons[i].joinable()) {
// cons[i].join();
//}
//}
//}
