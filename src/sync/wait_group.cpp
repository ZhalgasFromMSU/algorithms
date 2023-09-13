#include <algo/sync/wait_group.hpp>

namespace algo {

    WaitGroup::WaitGroup(int init) noexcept
        : counter_{init}
    {}

    bool WaitGroup::Inc() noexcept {
        int cur = counter_;
        while (cur >= 0) {
            if (counter_.compare_exchange_strong(cur, cur + 1)) {
                return true;
            }
        }
        return false;
    }

    bool WaitGroup::Dec() noexcept {
        int cur = counter_;
        while (cur != 0 && cur != negative_zero_) {
            if ((cur > 0 && counter_.compare_exchange_strong(cur, cur - 1)) ||
                (cur < -1 && counter_.compare_exchange_strong(cur, cur + 1)))
            {
                return true;
            } else if (cur == -1 &&
                       counter_.compare_exchange_strong(cur, negative_zero_))
            {
                counter_.notify_all();
                return true;
            }
        }
        return false;
    }

    void WaitGroup::Block() noexcept {
        int cur = counter_;
        while (cur >= 0) {
            if (cur == 0) {
                counter_.compare_exchange_strong(cur, negative_zero_);
                counter_.notify_all();
            } else {
                counter_.compare_exchange_strong(cur, -cur);
            }
        }
    }

    void WaitGroup::BlockAndWait() noexcept {
        Block();
        int cur = counter_;
        while (cur != negative_zero_) {
            counter_.wait(cur);
            cur = counter_;
        }
    }

    bool WaitGroup::Blocked() const noexcept {
        return counter_ < 0;
    }

    bool WaitGroup::Waited() const noexcept {
        return counter_ == negative_zero_;
    }

} // namespace algo
