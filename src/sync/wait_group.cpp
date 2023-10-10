#include <algo/sync/wait_group.hpp>
#include <atomic>

namespace algo {

WaitGroup::WaitGroup(int init) noexcept
    : counter_{init} {
}

bool WaitGroup::Inc() noexcept {
  int cur = counter_.load(std::memory_order_relaxed);
  while (cur >= 0) {
    if (counter_.compare_exchange_strong(cur, cur + 1,
                                         std::memory_order_relaxed)) {
      if (cur == 0) {
        counter_.notify_one();
      }

      return true;
    }
  }
  return false;
}

bool WaitGroup::Dec() noexcept {
  int cur = counter_.load(std::memory_order_relaxed);
  while (cur != 0 && cur != negative_zero_) {
    if (cur == -1 && counter_.compare_exchange_strong(
                         cur, negative_zero_, std::memory_order_relaxed)) {
      counter_.notify_all();
      return true;
    } else {
      int next_cur;
      if (cur > 0) {
        next_cur = cur - 1;
      } else { // cur < -1
        next_cur = cur + 1;
      }

      if (counter_.compare_exchange_strong(cur, next_cur,
                                           std::memory_order_relaxed)) {
        return true;
      }
    }
  }

  return false;
}

bool WaitGroup::WaitAndDec() noexcept {
  int cur = counter_.load(std::memory_order_relaxed);
  while (cur != negative_zero_) {
    if (cur == 0) {
      counter_.wait(cur, std::memory_order_relaxed);
      cur = counter_.load(std::memory_order_relaxed);
    } else if (cur == -1 &&
               counter_.compare_exchange_strong(cur, negative_zero_,
                                                std::memory_order_relaxed)) {
      counter_.notify_all();
      return true;
    } else {
      int next_cur;
      if (cur > 0) {
        next_cur = cur - 1;
      } else { // cur < -1
        next_cur = cur + 1;
      }

      if (counter_.compare_exchange_strong(cur, next_cur,
                                           std::memory_order_relaxed)) {
        return true;
      }
    }
  }
  return false;
}

void WaitGroup::Block() noexcept {
  int cur = counter_.load(std::memory_order_relaxed);
  while (cur >= 0) {
    if (cur == 0) {
      counter_.compare_exchange_strong(cur, negative_zero_,
                                       std::memory_order_relaxed);
      counter_.notify_all();
    } else {
      counter_.compare_exchange_strong(cur, -cur, std::memory_order_relaxed);
    }
  }
}

void WaitGroup::BlockAndWait() noexcept {
  Block();
  int cur = counter_.load(std::memory_order_relaxed);
  while (cur != negative_zero_) {
    counter_.wait(cur, std::memory_order_relaxed);
    cur = counter_.load(std::memory_order_relaxed);
  }
}

bool WaitGroup::Blocked() const noexcept {
  return counter_.load(std::memory_order_relaxed) < 0;
}

bool WaitGroup::Finished() const noexcept {
  return counter_.load(std::memory_order_relaxed) == negative_zero_;
}

} // namespace algo
