#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <tuple>

namespace algo {

    namespace mask {
        static constexpr int64_t kFirstHalf = -1ull >> 32 << 32; // 111000
        static constexpr int64_t kSecondHalf = -1ull ^ kFirstHalf; // 000111
    }

    constexpr bool IsPrime(int64_t number) noexcept {
        if (number < 2) {
            return false;
        }

        for (int64_t i = 2; i * i <= number; ++i) {
            if (number % i == 0) {
                return false;
            }
        }
        return true;
    }

    template<int64_t modulo>
    constexpr int64_t Normalize(int64_t x) noexcept {
        if (x < 0) {
            if (-x > modulo) {
                x %= modulo;
            }
            x = modulo + x;
        } else if (x >= modulo) {
            x %= modulo;
        }
        return x;
    }

    template<int64_t modulo>
    constexpr int64_t Normalize(uint64_t x) noexcept {
        if (x >= modulo) {
            x %= modulo;
        }
        return x;
    }

    template<int64_t modulo>
    constexpr int64_t Add(int64_t x, int64_t y) noexcept {
        x = Normalize<modulo>(x);
        y = Normalize<modulo>(y);
        if (modulo - y > x) { // going to overflow
            return Normalize<modulo>(x - modulo + y);
        } else {
            return x + y;
        }
    }

    template<int64_t modulo, int64_t maxModulo = (1ull << 63) % modulo>
    constexpr int64_t Mul(int64_t x, int64_t y) noexcept {
        x = Normalize<modulo>(x); // >= 0
        y = Normalize<modulo>(y); // >= 0

        uint64_t xH = x & mask::kFirstHalf >> 32;
        uint64_t xL = x & mask::kSecondHalf;

        uint64_t yH = y & mask::kFirstHalf >> 32;
        uint64_t yL = y & mask::kSecondHalf;

        int64_t res = 0;
        res += Normalize<modulo>(xH * yH);

        return res;
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
