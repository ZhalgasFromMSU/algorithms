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

        // any number that fits into 32 byte integer has at most 9 different prime factors:
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
                if (Power<order>(i, (order - 1) / prime_factors[j]) == 1) {
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
    constexpr ModuloField<order, _>::ModuloField(int x) noexcept
        : val {x}
    {
        val = Normalize<order>(val);
    }

    template<int order, bool _>
    constexpr ModuloField<order, _>& ModuloField<order, _>::operator+=(const ModuloField<order, _>& rhs) noexcept {
        val = Normalize<order>(static_cast<long long int>(val) + rhs.val);
        return *this;
    }

    template<int order, bool _>
    constexpr ModuloField<order, _>& ModuloField<order, _>::operator-=(const ModuloField& rhs) noexcept {
        val = Normalize<order>(static_cast<long long int>(val) - rhs.val);
        return *this;
    }

    template<int order, bool _>
    constexpr ModuloField<order, _>& ModuloField<order, _>::operator*=(const ModuloField& rhs) noexcept {
        val = Normalize<order>(static_cast<long long int>(val) * rhs.val);
        return *this;
    }

    template<int order, bool _>
    constexpr ModuloField<order, _>& ModuloField<order, _>::operator/=(const ModuloField& rhs) noexcept {
        return *this *= rhs.ToPower(order - 2);
    }

    template<int order, bool _>
    constexpr ModuloField<order, _> ModuloField<order, _>::ToPower(int exp) const noexcept {
        return Power<order>(val, exp);
    }

    template<int order, bool _>
    constexpr ModuloField<order, _> operator+(const ModuloField<order, _>& lhs, const ModuloField<order, _>& rhs) noexcept {
        auto out = lhs;
        out += rhs;
        return out;
    }

    template<int order, bool _>
    constexpr ModuloField<order, _> operator-(const ModuloField<order, _>& lhs, const ModuloField<order, _>& rhs) noexcept {
        auto out = lhs;
        out -= rhs;
        return out;
    }

    template<int order, bool _>
    constexpr ModuloField<order, _> operator*(const ModuloField<order, _>& lhs, const ModuloField<order, _>& rhs) noexcept {
        auto out = lhs;
        out *= rhs;
        return out;
    }

    template<int order, bool _>
    constexpr ModuloField<order, _> operator/(const ModuloField<order, _>& lhs, const ModuloField<order, _>& rhs) noexcept {
        auto out = lhs;
        out /= rhs;
        return out;
    }

} // namespace algo
