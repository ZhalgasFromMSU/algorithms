#pragma once

#include "util.hpp"

#include <concepts>

namespace algo {

    template<int order, bool suppress_prime_check = false>
    class ModuloField {
        static_assert(order > 1);
        static_assert(suppress_prime_check || IsPrime(order));

    public:
        static constexpr int primitive() noexcept {
            return primitive_root_;
        }

        template<std::integral T>
        static constexpr int Normalize(T x) noexcept;

        static constexpr bool Eq(int x, int y) noexcept;

        static constexpr int Add(int x, int y) noexcept;
        static constexpr int Sub(int x, int y) noexcept;
        static constexpr int Mul(int x, int y) noexcept;
        static constexpr int Div(int x, int y) noexcept;

        static constexpr int Power(int x, int exp) noexcept;

    private:
        static constexpr int FindPrimitiveRoot() noexcept;

        static constexpr int primitive_root_ = FindPrimitiveRoot();
    };

    using GF_2 = ModuloField<2>;
    using GF_BIG = ModuloField<1'000'005'001>;

} // namespace algo
