#pragma once

#include <algo/assert.hpp>

#include <atomic>
#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

namespace algo {

/*
 * Lock free queue
 * spsc - single producer/single consumer mode
 * push_sync - if Push(A) finishes before Push(B) gets called,
 *             then A will be Pop-ed before B
 *             (this option can be ommited if buffer size is very big)
 */
template<typename T, bool spsc = false, bool push_sync = false>
class LfQueue {
  static_assert(std::is_nothrow_move_constructible_v<T>);

public:
  LfQueue(std::size_t max_size) noexcept;

  // Block until data can be pushed/popped
  void Push(T&& obj) noexcept;
  T Pop() noexcept;

  bool TryPush(T&& obj) noexcept;
  std::optional<T> TryPop() noexcept;

private:
  enum class Status : char {
    kAvail,
    kInProg,
    kDone,
  };

  struct Node {
    std::optional<T> object;
    std::atomic<Status> push_is = Status::kAvail;
    std::atomic<Status> pop_is = Status::kDone;
  };

  struct SyncNode {
    std::optional<T> object;
    std::atomic<size_t> node_epoch;
    std::atomic<bool> is_ready;
  };

  using Payload = std::conditional_t<spsc || !push_sync, Node, SyncNode>;
  std::vector<Payload> data_;

  std::atomic<std::size_t> push_ptr_ = -1; // first write should be
                                           // at 0-th index

  std::atomic<std::size_t> pop_ptr_ = -1; // points to an element before last
};

// Shortcuts
template<typename T>
using SpscQueue = LfQueue<T, true>;

// Implementation
template<typename T, bool spsc, bool p_sync>
LfQueue<T, spsc, p_sync>::LfQueue(std::size_t max_size) noexcept
    : data_(max_size + 1) {
}

template<typename T, bool spsc, bool p_sync>
void LfQueue<T, spsc, p_sync>::Push(T&& obj) noexcept {
  size_t push_ptr = push_ptr_.fetch_add(1, std::memory_order_relaxed) + 1;

  if constexpr (spsc) {
    Node& node = data_[push_ptr % data_.size()];

    node.push_is.wait(Status::kDone, std::memory_order_acquire);
    node.object.emplace(std::move(obj));
    node.push_is.store(Status::kDone, std::memory_order_relaxed);

    node.pop_is.store(Status::kAvail, std::memory_order_release);
    node.pop_is.notify_one();
  } else if constexpr (!p_sync) {
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
  } else {
    SyncNode& node = data_[push_ptr % data_.size()];

    std::size_t cur_epoch = push_ptr / data_.size();
    std::size_t node_epoch = node.node_epoch.load(std::memory_order_relaxed);
    while (cur_epoch > node_epoch) {
      node.node_epoch.wait(node_epoch, std::memory_order_relaxed);
      node_epoch = node.node_epoch.load(std::memory_order_relaxed);
    }
    assert(cur_epoch == node_epoch);

    node.is_ready.wait(true,
                       std::memory_order_acquire); // Always returns instantly
    node.object.emplace(std::move(obj));
    node.is_ready.store(true, std::memory_order_release);
    node.is_ready.notify_one();
  }
}

template<typename T, bool spsc, bool p_sync>
T LfQueue<T, spsc, p_sync>::Pop() noexcept {
  std::size_t pop_ptr = pop_ptr_.fetch_add(1, std::memory_order_relaxed) + 1;

  if constexpr (spsc) {
    Node& node = data_[pop_ptr % data_.size()];

    node.pop_is.wait(Status::kDone, std::memory_order_acquire);
    T ret{std::move(node.object).value()};
    node.pop_is.store(Status::kDone, std::memory_order_relaxed);

    node.push_is.store(Status::kAvail, std::memory_order_release);
    node.push_is.notify_one();
    return ret;
  } else if constexpr (!p_sync) {
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
  } else {
    SyncNode& node = data_[pop_ptr % data_.size()];

    std::size_t cur_epoch = pop_ptr / data_.size();
    std::size_t node_epoch = node.node_epoch.load(std::memory_order_acquire);
    while (cur_epoch > node_epoch) {
      node.node_epoch.wait(node_epoch, std::memory_order_acquire);
      node_epoch = node.node_epoch.load(std::memory_order_acquire);
    }
    assert(cur_epoch == node_epoch);

    node.is_ready.wait(false, std::memory_order_acquire);
    T ret{std::move(node.object).value()};
    node.is_ready.store(false, std::memory_order_release);

    node.node_epoch.fetch_add(1, std::memory_order_relaxed);
    node.node_epoch.notify_all();

    return ret;
  }
}

template<typename T, bool spsc, bool p_sync>
bool LfQueue<T, spsc, p_sync>::TryPush(T&& obj) noexcept {
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

    if constexpr (!p_sync) {
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
    } else {
      push_ptr += 1;
      SyncNode& node = data_[push_ptr % data_.size()];

      // Wait for turn
      std::size_t cur_epoch = push_ptr / data_.size();
      std::size_t node_epoch = node.node_epoch.load(std::memory_order_acquire);

      while (cur_epoch > node_epoch) {
        node.node_epoch.wait(node_epoch, std::memory_order_relaxed);
        node_epoch = node.node_epoch.load(std::memory_order_acquire);
      }
      assert(cur_epoch == node_epoch);

      node.object.emplace(std::move(obj));
      node.is_ready.store(true, std::memory_order_release);
      node.is_ready.notify_one();
    }
  }
  return true;
}

template<typename T, bool spsc, bool p_sync>
std::optional<T> LfQueue<T, spsc, p_sync>::TryPop() noexcept {
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

    if constexpr (!p_sync) {
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
    } else {
      pop_ptr += 1;
      SyncNode& node = data_[pop_ptr % data_.size()];

      // Wait for turn
      std::size_t cur_epoch = pop_ptr / data_.size();
      std::size_t node_epoch = node.node_epoch.load(std::memory_order_acquire);
      while (cur_epoch > node_epoch) {
        node.node_epoch.wait(node_epoch, std::memory_order_relaxed);
        node_epoch = node.node_epoch.load(std::memory_order_acquire);
      }
      assert(cur_epoch == node_epoch);

      node.is_ready.wait(false, std::memory_order_acquire);
      ret = std::move(node.object);
      node.is_ready.store(false, std::memory_order_relaxed);

      node.node_epoch.fetch_add(1, std::memory_order_release);
      node.node_epoch.notify_all();
    }
  }
  assert(ret.has_value());
  return ret;
}

} // namespace algo
