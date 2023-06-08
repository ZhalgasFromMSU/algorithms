#pragma once

#include "util.hpp"

#include <concepts>

namespace algo {

    template<int order, bool suppress_prime_check = false>
    class ModuloField {
        static_assert(order > 1);
        static_assert(suppress_prime_check || IsPrime(order));

    public:
        static constexpr ModuloField primitive() noexcept {
            return kPrimitiveRoot;
        }

        constexpr ModuloField() noexcept = default;
        constexpr ModuloField(int x) noexcept;

        constexpr ModuloField& operator+=(const ModuloField&) noexcept;
        constexpr ModuloField& operator-=(const ModuloField&) noexcept;
        constexpr ModuloField& operator*=(const ModuloField&) noexcept;
        constexpr ModuloField& operator/=(const ModuloField&) noexcept;

        constexpr bool operator==(const ModuloField&) const noexcept = default;

        constexpr ModuloField ToPower(int exp) const noexcept;

        constexpr int value() const noexcept {
            return val;
        }

    private:
        static constexpr int FindPrimitiveRoot() noexcept;

        static constexpr int kPrimitiveRoot = FindPrimitiveRoot();
        int val = 0;
    };

    using GF_2 = ModuloField<2>;
    using GF_BIG = ModuloField<1'000'005'001>;

} // namespace algo
