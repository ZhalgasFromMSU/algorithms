#pragma once

#include <concepts>

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

    template<int order>
    constexpr int Normalize(std::integral auto x) noexcept {
        if (x < 0) {
            if (-x > order) {
                x %= order;
            }
            x = order + x;
        } else if (x >= order) {
            x %= order;
        }
        return x;
    }

    template<int order>
    constexpr int Power(int x, int exp) noexcept {
        if (exp >= order - 1) {
            exp %= order - 1;
        }

        if (exp == 0) {
            return 1;
        }

        long long int tmp = Power<order>(x, exp / 2);
        tmp = Normalize<order>(tmp * tmp);
        if (exp & 1) {
            tmp = Normalize<order>(tmp * x);
        }
        return tmp;
    }

} // namespace algo
