#pragma once

#include <algo/assert.hpp>

#include <atomic>
#include <cassert>
#include <optional>
#include <vector>

namespace algo {

/*
 * Lock free queue
 * spsc - single producer/single consumer mode
 */
template<typename T, bool spsc = false>
class LfQueue {
  static_assert(std::is_nothrow_move_constructible_v<T>);

public:
  LfQueue(std::size_t max_size) noexcept;

  // Block until data can be pushed/popped
  void Push(T obj) noexcept;
  T Pop() noexcept;

  bool TryPush(T obj) noexcept;
  std::optional<T> TryPop() noexcept;

private:
  enum class Status : char {
    kAvail,
    kInProg,
    kDone,
  };

  struct Node {
    std::optional<T> object;
    std::atomic<Status> push_is = Status::kAvail; // push is available
    std::atomic<Status> pop_is = Status::kDone;   // push is done
  };

  std::vector<Node> data_;

  std::atomic<std::size_t> push_ptr_ = -1; // first write should be
                                           // at 0-th index

  std::atomic<std::size_t> pop_ptr_ = -1; // points to an element before last
};

// Shortcuts
template<typename T>
using SpscQueue = LfQueue<T, true>;

// Implementation
template<typename T, bool spsc>
LfQueue<T, spsc>::LfQueue(std::size_t max_size) noexcept
    : data_(max_size + 1) {
}

template<typename T, bool spsc>
void LfQueue<T, spsc>::Push(T obj) noexcept {
  size_t push_ptr = push_ptr_.fetch_add(1, std::memory_order_relaxed) + 1;

  if constexpr (spsc) {
    Node& node = data_[push_ptr % data_.size()];

    node.push_is.wait(Status::kDone, std::memory_order_acquire);
    node.object.emplace(std::move(obj));
    node.push_is.store(Status::kDone, std::memory_order_relaxed);

    node.pop_is.store(Status::kAvail, std::memory_order_release);
    node.pop_is.notify_one();
  } else {
    Node& node = data_[push_ptr % data_.size()];

    // Acquire lock
    Status status = node.push_is.load(std::memory_order_relaxed);
    while (true) {
      while (status != Status::kAvail) {
        node.push_is.wait(status, std::memory_order_relaxed);
        status = node.push_is.load(std::memory_order_relaxed);
      }

      if (node.push_is.compare_exchange_strong(status, Status::kInProg,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed)) {
        break;
      }
    }

    // Emplace element
    node.object.emplace(std::move(obj));
    node.push_is.store(Status::kDone, std::memory_order_relaxed);

    // Wake one thread that wants to Pop in this position
    node.pop_is.store(Status::kAvail, std::memory_order_release);
    node.pop_is.notify_one();
  }
}

template<typename T, bool spsc>
T LfQueue<T, spsc>::Pop() noexcept {
  std::size_t pop_ptr = pop_ptr_.fetch_add(1, std::memory_order_relaxed) + 1;

  if constexpr (spsc) {
    Node& node = data_[pop_ptr % data_.size()];

    node.pop_is.wait(Status::kDone, std::memory_order_acquire);
    T ret{std::move(node.object).value()};
    node.pop_is.store(Status::kDone, std::memory_order_relaxed);

    node.push_is.store(Status::kAvail, std::memory_order_release);
    node.push_is.notify_one();
    return ret;
  } else {
    Node& node = data_[pop_ptr % data_.size()];

    // Acquire lock
    Status status = node.pop_is.load(std::memory_order_relaxed);
    while (true) {
      while (status != Status::kAvail) {
        node.pop_is.wait(status, std::memory_order_relaxed);
        status = node.pop_is.load(std::memory_order_relaxed);
      }

      if (node.pop_is.compare_exchange_strong(status, Status::kInProg,
                                              std::memory_order_acquire,
                                              std::memory_order_relaxed)) {
        break;
      }
    }

    // Pop data
    T ret{std::move(node.object).value()};
    node.pop_is.store(Status::kDone, std::memory_order_relaxed);

    // Wake up one waiting thread
    node.push_is.store(Status::kAvail, std::memory_order_release);
    node.push_is.notify_one();
    return ret;
  }
}

template<typename T, bool spsc>
bool LfQueue<T, spsc>::TryPush(T obj) noexcept {
  if constexpr (spsc) {
    std::size_t push_ptr = push_ptr_.load(std::memory_order_relaxed);
    std::size_t pop_ptr = pop_ptr_.load(std::memory_order_relaxed);
    assert(push_ptr - pop_ptr < data_.size());
    if (push_ptr - pop_ptr == data_.size() - 1) {
      return false;
    }

    push_ptr_.store(++push_ptr, std::memory_order_relaxed);

    Node& node = data_[push_ptr % data_.size()];
    node.push_is.wait(Status::kDone, std::memory_order_acquire);
    node.object.emplace(std::move(obj));
    node.push_is.store(Status::kDone, std::memory_order_relaxed);

    node.pop_is.store(Status::kAvail, std::memory_order_release);
    node.pop_is.notify_one();
  } else {
    std::size_t push_ptr = push_ptr_.load(std::memory_order_relaxed);

    // Increment push_ptr
    while (true) {
      std::size_t pop_ptr = pop_ptr_.load(std::memory_order_relaxed);
      assert(push_ptr - pop_ptr < data_.size());
      if (push_ptr - pop_ptr == data_.size() - 1) {
        return false;
      }

      if (push_ptr_.compare_exchange_strong(push_ptr, push_ptr + 1,
                                            std::memory_order_relaxed)) {
        break;
      }
    }

    Node& node = data_[(push_ptr + 1) % data_.size()];

    // Acquire a lock for a node
    Status status = node.push_is.load(std::memory_order_relaxed);
    while (true) {
      while (status != Status::kAvail) {
        node.push_is.wait(status, std::memory_order_relaxed);
        status = node.push_is.load(std::memory_order_relaxed);
      }

      if (node.push_is.compare_exchange_strong(status, Status::kInProg,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed)) {
        break;
      }
    }

    node.object.emplace(std::move(obj));
    node.push_is.store(Status::kDone, std::memory_order_relaxed);

    node.pop_is.store(Status::kAvail, std::memory_order_release);
    node.pop_is.notify_one();
  }
  return true;
}

template<typename T, bool spsc>
std::optional<T> LfQueue<T, spsc>::TryPop() noexcept {
  std::optional<T> ret;
  if constexpr (spsc) {
    std::size_t pop_ptr = pop_ptr_.load(std::memory_order_relaxed);
    std::size_t push_ptr = push_ptr_.load(std::memory_order_relaxed);
    assert(push_ptr - pop_ptr < data_.size());
    if (push_ptr == pop_ptr) {
      return std::nullopt;
    }

    pop_ptr_.store(++push_ptr, std::memory_order_relaxed);

    Node& node = data_[pop_ptr % data_.size()];
    node.pop_is.wait(Status::kDone, std::memory_order_acquire);
    ret = std::move(node.object);
    node.pop_is.store(Status::kDone, std::memory_order_relaxed);

    node.push_is.store(Status::kAvail, std::memory_order_release);
    node.push_is.notify_one();
  } else {
    std::size_t pop_ptr = pop_ptr_.load(std::memory_order_relaxed);

    while (true) {
      std::size_t push_ptr = push_ptr_.load(std::memory_order_relaxed);
      assert(push_ptr - pop_ptr < data_.size());
      if (push_ptr == pop_ptr) {
        return std::nullopt;
      }

      if (pop_ptr_.compare_exchange_strong(pop_ptr, pop_ptr + 1,
                                           std::memory_order_relaxed)) {
        break;
      }
    }

    Node& node = data_[(pop_ptr + 1) % data_.size()];

    // Acquire a lock for a node
    Status status = node.pop_is.load(std::memory_order_relaxed);
    while (true) {
      while (status != Status::kAvail) {
        node.pop_is.wait(status, std::memory_order_relaxed);
        status = node.pop_is.load(std::memory_order_relaxed);
      }

      if (node.pop_is.compare_exchange_strong(status, Status::kInProg,
                                              std::memory_order_acquire,
                                              std::memory_order_relaxed)) {
        break;
      }
    }

    ret = std::move(node.object);
    node.pop_is.store(Status::kDone, std::memory_order_relaxed);

    node.push_is.store(Status::kAvail, std::memory_order_release);
    node.push_is.notify_one();
  }
  assert(ret.has_value());
  return ret;
}

} // namespace algo
