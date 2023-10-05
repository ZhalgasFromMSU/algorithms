#pragma once

#include <concurrentqueue.h>

#include <cassert>
#include <condition_variable>
#include <deque>
#include <latch>
#include <thread>
#include <vector>

namespace algo {

class Benchmarker {
public:
  Benchmarker(std::size_t num_prods, std::size_t num_cons) noexcept
      : prods_(num_prods)
      , cons_(num_cons)
      , latch(num_prods + num_cons) {
    assert(num_prods > 0 && num_cons > 0 && "No threads created");
  }

  template<typename F>
  void SetProducerFunc(F func) noexcept {
    for (auto& t : prods_) {
      t = std::jthread{[&, func] {
        start.wait(false, std::memory_order_relaxed);
        func();
        latch.count_down();
      }};
    }
  }

  template<typename F>
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

template<typename T>
class ConcurrentQueueAdaptor : protected moodycamel::ConcurrentQueue<T> {
public:
  using Base = moodycamel::ConcurrentQueue<T>;
  using Base::Base;

  void Push(T&& item) {
    if (!Base::enqueue(std::move(item))) {
      std::terminate();
    }
  }

  T Pop() {
    T ret;
    while (!Base::try_dequeue(ret)) {
    }
    return ret;
  }
};

template<typename T>
class LockingQueue {
public:
  LockingQueue(std::size_t size_hint) {
  }

  void Push(T&& item) {
    std::unique_lock lock{mutex_};
    data_.push_back(std::move(item));
    cv_.notify_one();
  }

  T Pop() {
    std::unique_lock lock{mutex_};
    cv_.wait(lock, [&] { return !data_.empty(); });
    T ret = std::move(data_.front());
    data_.pop_front();
    return ret;
  }

private:
  std::deque<T> data_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

} // namespace algo
