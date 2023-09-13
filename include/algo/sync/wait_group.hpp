#pragma once

#include <atomic>
#include <limits>

namespace algo {

    class WaitGroup {
    public:
        WaitGroup() noexcept = default;
        WaitGroup(int init) noexcept;

        bool Inc() noexcept;
        bool Dec() noexcept;
        void Block() noexcept;
        void BlockAndWait() noexcept;

        bool Blocked() const noexcept;
        bool Waited() const noexcept;

    private:
        static constexpr int negative_zero_ = std::numeric_limits<int>::min();

        std::atomic<int> counter_ = 0;
    };

} // namespace algo
