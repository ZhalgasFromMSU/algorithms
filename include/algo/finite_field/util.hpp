#pragma once

#include "concepts.hpp"

#include <array>
#include <span>

namespace algo {

    template<std::integral T>
    constexpr bool IsPrime(T number, double degree_of_certainty = 0.999999999) noexcept {
        // https://en.wikipedia.org/wiki/Baillie–PSW_primality_test






        return true;
    }

    template<Integral T, T modulo>
    constexpr void Normalize(T& x) noexcept {
        if (x < 0) {
            if (-x > modulo) {
                x %= modulo;
            }
            x = modulo + x;
        } else if (x >= modulo) {
            x %= modulo;
        }
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
