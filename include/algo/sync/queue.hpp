#pragma once

#include <algo/assert.hpp>

#include <atomic>
#include <optional>
#include <vector>

namespace algo {

template<typename T>
class LfQueue {
  static_assert(std::is_nothrow_move_constructible_v<T>,
                "Can't use noexcept Pop");

public:
  LfQueue(size_t max_size) noexcept;

  bool Push(T&& obj) noexcept;
  std::optional<T> Pop() noexcept;

private:
  struct Node {
    std::optional<T> object;
    std::atomic<size_t> write_order = 0;
    std::atomic<size_t> read_order = 0;
  };

  std::vector<Node> data_;
  std::atomic<size_t> push_ptr_ = -1; // first write should be at 0-th index
  std::atomic<size_t> pop_ptr_ = -1;  // points to an element before last
};

// Implementation
template<typename T>
LfQueue<T>::LfQueue(size_t max_size) noexcept
    : data_(max_size + 1) {
}

template<typename T>
bool LfQueue<T>::Push(T&& obj) noexcept {
  size_t idx;
  size_t push_ptr = push_ptr_;
  while (true) {
    size_t new_ptr = push_ptr + 1;
    if (new_ptr - pop_ptr_ >= data_.size()) {
      return false;
    }

    if (push_ptr_.compare_exchange_strong(push_ptr, new_ptr)) {
      idx = new_ptr;
      break;
    }
  }

  Node& node = data_[idx % data_.size()];
  size_t current_order = idx / data_.size();

  while (current_order > node.write_order) {
    // Spin-wait for our turn to write to this position
  }

  while (current_order > node.read_order) {
    // Spin-wait until data was loaded from this position
  }

  node.object = std::move(obj);
  ASSERT(node.write_order.fetch_add(1) == current_order);
  return true;
}

template<typename T>
std::optional<T> LfQueue<T>::Pop() noexcept {
  size_t idx;
  size_t pop_ptr = pop_ptr_;
  while (true) {
    if (pop_ptr == push_ptr_) {
      return std::nullopt;
    }

    if (pop_ptr_.compare_exchange_strong(pop_ptr, pop_ptr + 1)) {
      idx = pop_ptr + 1;
      break;
    }
  }

  Node& node = data_[idx % data_.size()];
  size_t current_order = idx / data_.size();

  while (current_order > node.read_order) {
    // Spin-wait for our turn to read data from this position
  }

  while (current_order == node.write_order) {
    // Spin-wait until something appears in this position
  }

  std::optional<T> ret = std::move(node.object);
  ASSERT(node.read_order.fetch_add(1) == current_order);
  return ret;
}

} // namespace algo
