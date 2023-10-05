#include "helpers.hpp"

#include <algo/sync/queue.hpp>
#include <concurrentqueue.h>

#include <benchmark/benchmark.h>

#include <chrono>
#include <deque>
#include <latch>
#include <thread>

template<typename T>
struct QueueFactory {
  using Type = T;

  static T New(std::size_t max_size) noexcept {
    return T(max_size);
  }
};

template<typename QueueFactory, bool spsc>
static void BM_QueueInsertion(benchmark::State& state) noexcept {
  constexpr std::size_t max = 1'000;
  std::size_t producers = spsc ? 1 : 3;
  std::size_t consumers = spsc ? 1 : 3;

  for (auto _ : state) {
    algo::Benchmarker bm{producers, consumers};

    std::atomic<std::size_t> n_push, n_pop;
    std::vector<std::atomic_flag> flags(max);
    auto q = QueueFactory::New(state.range(0));
    bm.SetConsumerFunc([&] {
      while (true) {
        std::size_t cur = n_push.fetch_add(1, std::memory_order_relaxed);
        if (cur >= max) {
          return;
        }

        q.Push(std::move(cur));
      }
    });

    bm.SetProducerFunc([&] {
      while (true) {
        std::size_t cur = n_pop.fetch_add(1, std::memory_order_relaxed);
        if (cur >= max) {
          return;
        }

        if (flags.at(q.Pop()).test_and_set(std::memory_order_relaxed)) {
          std::terminate();
        }
      }
    });

    state.SetIterationTime(std::move(bm).Run());
    for (const auto& flag : flags) {
      if (!flag.test(std::memory_order_relaxed)) {
        std::terminate();
      }
    }
  }
}

// Multiple producers multiple consumers
BENCHMARK(BM_QueueInsertion<QueueFactory<algo::LfQueue<std::size_t>>, false>)
    ->RangeMultiplier(10)
    ->Range(10, 1000);

BENCHMARK(
    BM_QueueInsertion<QueueFactory<algo::LockingQueue<std::size_t>>, false>)
    ->RangeMultiplier(10)
    ->Range(10, 1000);

BENCHMARK(BM_QueueInsertion<
              QueueFactory<algo::ConcurrentQueueAdaptor<std::size_t>>, false>)
    ->RangeMultiplier(10)
    ->Range(10, 1000);

// Single producer single consumer
BENCHMARK(
    BM_QueueInsertion<QueueFactory<algo::LfQueue<std::size_t, true>>, true>)
    ->RangeMultiplier(10)
    ->Range(10, 1000);

BENCHMARK(
    BM_QueueInsertion<QueueFactory<algo::LockingQueue<std::size_t>>, true>)
    ->RangeMultiplier(10)
    ->Range(10, 1000);

BENCHMARK(BM_QueueInsertion<
              QueueFactory<algo::ConcurrentQueueAdaptor<std::size_t>>, true>)
    ->RangeMultiplier(10)
    ->Range(10, 1000);
