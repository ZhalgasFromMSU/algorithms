#pragma once

namespace algo {

    constexpr bool IsPrime(int number) noexcept {
        if (number < 2) {
            return false;
        }

        for (int i = 2; i * i <= number; ++i) {
            if (number % i == 0) {
                return false;
            }
        }
        return true;
    }

} // namespace algo
