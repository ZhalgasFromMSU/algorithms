module;

#include <thread>
#include <vector>

export module algo:thread_pool;

import :lfqueue;
import :wait_group;

namespace algo {

  export template<std::invocable Task>
  class ThreadPool {
  public:
    ThreadPool(std::size_t max_queue_size, std::size_t num_workers) noexcept
        : queue_{max_queue_size, num_workers, num_workers}
        , workers_(num_workers) {
    }

    bool Enqueue(Task&& task) {
      if (!wg_.TryInc()) {
        return false;
      }
      queue_.Push(std::move(task));
      return true;
    }

    void Start() noexcept {
      for (auto& worker : workers_) {
        worker = std::jthread{[&] {
          while (wg_.Dec()) {
            queue_.Pop()();
          }
        }};
      }
    }

    void Stop() noexcept {
      wg_.Block();
      wg_.Wait();
    }

  private:
    LfQueue<Task> queue_;
    WaitGroup wg_;
    std::vector<std::jthread> workers_;
  };

} // namespace algo
