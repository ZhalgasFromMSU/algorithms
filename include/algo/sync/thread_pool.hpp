#pragma once

#include "queue.hpp"
#include "wait_group.hpp"

#include <latch>
#include <thread>

namespace algo {

template<std::invocable Task>
class ThreadPool {
public:
  ThreadPool(std::size_t threads, std::size_t queue_size) noexcept;

  // Create threads and run them
  void Start() noexcept;

  // Enqueue task. Returns false if Stop was called
  bool Enqueue(Task task) noexcept;

  // Stop enqueue-ing and wait for remaining tasks to finish
  void Stop() noexcept;

private:
  std::vector<std::jthread> threads_;
  LfQueue<Task> tasks_;
  WaitGroup wg_;
  std::latch stop_latch_;
};

// Implementation
template<std::invocable Task>
ThreadPool<Task>::ThreadPool(std::size_t threads,
                             std::size_t queue_size) noexcept
    : threads_(threads)
    , tasks_{queue_size}
    , stop_latch_(threads) {
}

template<std::invocable Task>
void ThreadPool<Task>::Start() noexcept {
  for (auto& thread : threads_) {
    thread = std::jthread{[&] {
      while (wg_.WaitAndDec()) {
        Task t = tasks_.Pop();
        t();
      }

      stop_latch_.count_down();
    }};
  }
}

template<std::invocable Task>
bool ThreadPool<Task>::Enqueue(Task task) noexcept {
  if (wg_.Inc()) {
    tasks_.Push(std::move(task));
    return true;
  }

  return false;
}

template<std::invocable Task>
void ThreadPool<Task>::Stop() noexcept {
  wg_.BlockAndWait();
  stop_latch_.wait();
}

} // namespace algo
