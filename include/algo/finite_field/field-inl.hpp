#include "field.hpp"

#include <array>
#include <exception>

namespace algo {

    template<int order, bool _>
    constexpr int ModuloField<order, _>::FindPrimitiveRoot() noexcept {
        // Algorithm: https://www.geeksforgeeks.org/primitive-root-of-a-prime-number-n-modulo-n/
        if (order == 2) [[unlikely]] {
            return 1;
        } else if (order == 3) [[unlikely]] {
            return 2;
        }

        // any number that fits into 32 byte integer has at most 9 prime different factors:
        // 2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23 = 223,092,870 (_ * 29 > max_int)
        std::array<int, 9> prime_factors;

        // find prime factors
        std::size_t id = 0;
        int phi = order - 1;
        for (int i = 2; i * i <= phi; ++i) {
            if (phi % i == 0) {
                prime_factors.at(id++) = i;
                while (phi % i == 0) {
                    phi /= i;
                }
            }
        }
        if (phi > 1) {
            prime_factors.at(id++) = phi;
        }

        // find primitive root
        for (int i = 2; i < order; ++i) {
            std::size_t j = 0;
            while (j < id) {
                if (Power(i, (order - 1) / prime_factors[j]) == 1) {
                    break;
                }
                ++j;
            }
            if (j == id) {
                return i;
            }
        }

        std::terminate();
    }

    template<int order, bool _>
    template<std::integral T>
    constexpr int ModuloField<order, _>::Normalize(T x) noexcept {
        if (x < 0) {
            if (-x >= order) {
                x %= order;
            }
            x = order - x;
        } else if (x >= order) {
            x %= order;
        }
        return x;
    }

    template<int order, bool _>
    constexpr bool ModuloField<order, _>::Eq(int x, int y) noexcept {
        return Normalize(x) == Normalize(y);
    }

    template<int order, bool _>
    constexpr int ModuloField<order, _>::Add(int x, int y) noexcept {
        return Normalize(static_cast<long long int>(x) + y);
    }

    template<int order, bool _>
    constexpr int ModuloField<order, _>::Sub(int x, int y) noexcept {
        return Normalize(static_cast<long long int>(x) - y);
    }

    template<int order, bool _>
    constexpr int ModuloField<order, _>::Mul(int x, int y) noexcept {
        return Normalize(static_cast<long long int>(x) * y);
    }

    template<int order, bool _>
    constexpr int ModuloField<order, _>::Div(int x, int y) noexcept {
        if (y == 0) {
            std::terminate();
        }
        if (y < 0) {
            y = Normalize(y);
        }
        return Normalize(Mul(x, Power(y, order - 2)));
    }

    template<int order, bool _>
    constexpr int ModuloField<order, _>::Power(int x, int exp) noexcept {
        if (exp >= order - 1) {
            exp %= order - 1;
        }

        if (exp == 0) {
            return 1;
        }

        int tmp = Power(x, exp / 2);
        if (exp & 1) {
            return Mul(Mul(tmp, tmp), x);
        } else {
            return Mul(tmp, tmp);
        }

        return tmp;
    }

} // namespace algo
