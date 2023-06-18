#pragma once

#include "bigint.hpp"
#include <bit>

namespace algo {

    template<typename T, T modulo, T primitive_root>
    class ModuloField {
        static_assert(std::is_same_v<T, int64_t> || IsBigInt<T>::value);
    public:
        template<typename ...Args>
        constexpr ModuloField(Args&&... args) noexcept;

        constexpr bool operator==(const ModuloField&) const noexcept = default;

        constexpr ModuloField& operator+=(const ModuloField&) noexcept;
        constexpr ModuloField& operator-=(const ModuloField&) noexcept;
        constexpr ModuloField& operator*=(const ModuloField&) noexcept requires (IsBigInt<T>::value);
        constexpr ModuloField& operator*=(const ModuloField&) noexcept requires (std::is_same_v<T, int64_t>);
        constexpr ModuloField& operator/=(const ModuloField&) noexcept;

        constexpr ModuloField Power(T exp) const noexcept;
        constexpr ModuloField Reversed() const noexcept;

        T value;

    private:
        constexpr void Normalize() noexcept;
    };

    // specializations

    // impl
    template<typename T, T mod, T pr>
    template<typename ...Args>
    constexpr ModuloField<T, mod, pr>::ModuloField(Args&&... args) noexcept
        : value{std::forward<Args>(args)...}
    {
        Normalize();
    }

    template<typename T, T mod, T pr>
    constexpr ModuloField<T, mod, pr>& ModuloField<T, mod, pr>::operator+=(const ModuloField& rhs) noexcept {
        if (mod - rhs.value < value) {
            value += rhs.value - mod;
        } else {
            value += rhs.value;
        }
        Normalize();
        return *this;
    }

    template<typename T, T mod, T pr>
    constexpr ModuloField<T, mod, pr>& ModuloField<T, mod, pr>::operator-=(const ModuloField& rhs) noexcept {
        value -= rhs.value;
        Normalize();
        return *this;
    }

    template<typename T, T mod, T pr>
    constexpr ModuloField<T, mod, pr>& ModuloField<T, mod, pr>::operator*=(const ModuloField& rhs) noexcept requires (IsBigInt<T>::value) {
        if (value.words_count + rhs.value.words_count >= T::kWordCapacity) {
            BigInt<T::kWordCapacity * 2> ret {value.binary.begin(), value.binary.begin() + value.words_count};
            ret *= BigInt<T::kWordCapacity * 2>{rhs.value.binary.begin(), rhs.value.binary.begin() + rhs.value.words_count};
            ret %= BigInt<T::kWordCapacity * 2>{mod.binary.begin(), mod.binary.begin() + mod.words_count};
            value = {ret.binary.begin(), ret.binary.begin() + ret.words_count};
        } else {
            value *= rhs.value;
        }
        Normalize();
        return *this;
    }

    template<typename T, T mod, T pr>
    constexpr ModuloField<T, mod, pr>& ModuloField<T, mod, pr>::operator*=(const ModuloField& rhs) noexcept requires (std::is_same_v<T, int64_t>) {
        if (std::bit_width(value) + std::bit_width(rhs.value) > 63) {
            BigInt<4> ret {value};
            ret *= rhs.value;
            ret %= mod;
            value = (static_cast<int64_t>(ret.binary[1]) << 32) + ret.binary[0];
        } else {
            value *= rhs.value;
        }
        Normalize();
        return *this;
    }

    template<typename T, T mod, T pr>
    constexpr ModuloField<T, mod, pr>& ModuloField<T, mod, pr>::operator/=(const ModuloField& rhs) noexcept {
        return *this *= rhs.Reversed();
    }

    template<typename T, T mod, T pr>
    constexpr ModuloField<T, mod, pr> ModuloField<T, mod, pr>::Power(T exp) const noexcept {
        if (exp < 0) {
            return Reversed().Power(-exp);
        }

        if (exp == 0) {
            return 1;
        } else if (exp == 1) {
            return value;
        }

        ModuloField ret = Power(exp / 2);
        ret *= ret;
        if (exp % 2 == 1) {
            ret *= value;
        }

        return ret;
    }

    template<typename T, T mod, T pr>
    constexpr ModuloField<T, mod, pr> ModuloField<T, mod, pr>::Reversed() const noexcept {
        return Power(mod - 2);
    }

    template<typename T, T mod, T pr>
    constexpr void ModuloField<T, mod, pr>::Normalize() noexcept {
        if (value < 0) {
            if (-value >= mod) {
                value %= mod;
            }
            value += mod;
        } else if (value >= mod) {
            value %= mod;
        }
    }

} // namespace algo
