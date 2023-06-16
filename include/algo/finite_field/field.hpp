#pragma once

#include <concepts>
#include <cstdint>
#include <utility>

namespace algo {

    template<typename T>
    concept FieldFriendlyType = requires (T lhs, T rhs) {
        (lhs + rhs) -> T;
        (lhs - rhs) -> T;
        (lhs * rhs) -> T;
        (lhs % rhs) -> T;
        (-lhs) -> T;

        (lhs <=> rhs);
        (lhs == rhs);

        T{0};
        T{1};
    };

    template<FieldFriendlyType T, T modulo, T primitive_root>
    class Field {
    public:
        template<typename... Args>
            requires(std::is_constructible_v<T, Args...>)
        constexpr Field(Args&&... args) noexcept;

        constexpr Field& operator+=(const Field&) noexcept;
        constexpr Field& operator-=(const Field&) noexcept;
        constexpr Field& operator*=(const Field&) noexcept;
        constexpr Field& operator/=(const Field&) noexcept;

        constexpr bool operator==(const Field&) const noexcept;

        constexpr Field Power(std::size_t exp) const noexcept;

    private:
        constexpr void Normalize() noexcept;

        T Val;
    };

    // impl
    template<FieldFriendlyType T, T modulo, T primitive_root>
    template<typename... Args>
        requires(std::is_constructible_v<T, Args...>)
    constexpr Field<T, modulo, primitive_root>::Field(Args&&... args) noexcept
        : Val{std::forward<Args>(args)...}
    {
        Normalize();
    }

    template<FieldFriendlyType T, T modulo, T primitive_root>
    constexpr void Field<T, modulo, primitive_root>::Normalize() noexcept {
        if (Val < 0) {
            Val = -Val;
        }
        if (Val >= modulo) {
            Val %= modulo;
        }
    }

    template<FieldFriendlyType T, T modulo, T primitive_root>
    constexpr auto Field<T, modulo, primitive_root>::operator+=(const Field& other) noexcept -> Field& {
        if (modulo - Val < other.Val) {
            Val = Val - modulo + other.Val;
        }
        Normalize();
    }

    template<FieldFriendlyType T, T modulo, T primitive_root>
    constexpr auto Field<T, modulo, primitive_root>::operator-=(const Field& other) noexcept -> Field& {
        
    }

} // namespace algo
