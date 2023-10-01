#include <algo/sync/queue.hpp>

#include <benchmark/benchmark.h>

#include <chrono>
#include <latch>
#include <thread>

class Benchmarker {
public:
  Benchmarker(size_t num_prods, size_t num_cons) noexcept
      : prods_(num_prods)
      , cons_(num_cons)
      , latch(num_prods + num_cons) {
    assert(num_prods > 0 && num_const > 0 && "No threads created");
  }

  template <typename F>
  void SetProducerFunc(F func) noexcept {
    for (auto& t : prods_) {
      t = std::jthread{[&, func] {
        start.wait(false, std::memory_order_relaxed);
        func();
        latch.count_down();
      }};
    }
  }

  template <typename F>
  void SetConsumerFunc(F func) noexcept {
    for (auto& t : cons_) {
      t = std::jthread{[&, func] {
        start.wait(false, std::memory_order_relaxed);
        func();
        latch.count_down();
      }};
    }
  }

  // Returns number of seconds
  double Run() && noexcept {
    auto begin = std::chrono::high_resolution_clock::now();
    start.test_and_set(std::memory_order_relaxed);
    start.notify_all();
    latch.wait();

    return std::chrono::duration_cast<std::chrono::duration<double>>(
               std::chrono::high_resolution_clock::now() - begin)
        .count();
  }

private:
  std::vector<std::jthread> prods_;
  std::vector<std::jthread> cons_;

  std::atomic_flag start;
  std::latch latch;
};

template <typename Queue>
static void BM_QueueInsertion(benchmark::State& state) {
  constexpr size_t max = 100'000;
  for (auto _ : state) {
    Benchmarker bm{std::thread::hardware_concurrency() / 2 /* consumers */,
                   std::thread::hardware_concurrency() / 2 /* producers */};

    std::atomic<size_t> n_push, n_pop;
    Queue q(state.range(0));
    bm.SetConsumerFunc([&] {
      while (true) {
        size_t cur = n_push.fetch_add(1, std::memory_order_relaxed);
        if (cur >= max) {
          return;
        }

        while (!q.Push(std::move(cur))) {
        }
      }
    });

    bm.SetProducerFunc([&] {
      while (true) {
        size_t cur = n_pop.fetch_add(1, std::memory_order_relaxed);
        if (cur >= max) {
          return;
        }

        while (!q.Pop()) {
        }
      }
    });

    state.SetIterationTime(std::move(bm).Run());
  }
}

BENCHMARK(BM_QueueInsertion<algo::LfQueue<size_t>>)
    ->RangeMultiplier(10)
    ->Range(10, 1000);
