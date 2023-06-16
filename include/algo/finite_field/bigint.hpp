#pragma once

#include <iostream>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <bit>

namespace algo {

    template<size_t bit_size>
    class BigInt;

    template<typename T>
    struct IsBigInt : std::false_type {};

    template<size_t bit_size>
    struct IsBigInt<BigInt<bit_size>> : std::true_type {};

    template<size_t bit_size>
    class BigInt {
        static_assert(bit_size > 0);

        static constexpr uint32_t kMaxInt = std::numeric_limits<uint32_t>::max();
        static constexpr size_t kWordCount = (bit_size + 31) / 32;
        static constexpr uint32_t kFirstWordMask = bit_size % 32 == 0 ? kMaxInt : (1u << bit_size % 32) - 1u;

    public:
        constexpr BigInt() noexcept = default;
        constexpr BigInt(int64_t integer) noexcept;
        constexpr BigInt(std::string_view str) noexcept;

        constexpr bool operator==(const BigInt&) const noexcept;
        constexpr auto operator<=>(const BigInt&) const noexcept;

        constexpr BigInt& operator<<=(size_t) noexcept;
        constexpr BigInt& operator>>=(size_t) noexcept;
        constexpr BigInt operator<<(size_t) const noexcept;
        constexpr BigInt operator>>(size_t) const noexcept;


        constexpr BigInt operator-() const noexcept;
        constexpr BigInt& operator+=(const BigInt&) noexcept;
        constexpr BigInt& operator-=(const BigInt&) noexcept;
        constexpr BigInt& operator*=(const BigInt&) noexcept;
        constexpr BigInt& operator/=(const BigInt&) noexcept;
        constexpr BigInt& operator%=(const BigInt&) noexcept;

        constexpr BigInt operator~() const noexcept;
        constexpr BigInt& operator^=(const BigInt&) noexcept;
        constexpr BigInt& operator&=(const BigInt&) noexcept;
        constexpr BigInt& operator|=(const BigInt&) noexcept;

        constexpr BigInt Power(size_t exp) const noexcept;

        constexpr void Print() const noexcept {
            if (is_positive_) {
                std::cerr << "+ ";
            } else {
                std::cerr << "- ";
            }
            for (size_t i = 0; i < kWordCount; ++i) {
                uint32_t n = 1u << 31;
                while (n != 0) {
                    std::cerr << (binary_[kWordCount - 1 - i] & n ? 1 : 0);
                    n >>= 1;
                }
            }
            std::cerr << std::endl;
        }

    // private:
        constexpr std::pair<BigInt, BigInt> DivideUnsigned(const BigInt&) const noexcept; // returns div and remainder

        bool is_positive_ = true;
        std::array<uint32_t, kWordCount> binary_ = {}; // number is storred right to left, e.g. most significant bits are at the end of an array
                                                       // can't use bitset here, because not constexp (since C++23)
    };

    // impl
    template<size_t bit_size>
    constexpr BigInt<bit_size>::BigInt(int64_t from) noexcept {
        if (from < 0) {
            is_positive_ = false;
            from = -from;
        }

        binary_[0] = from & ((1ull << 32) - 1);
        if (kWordCount > 1) {
            binary_[1] = from >> 32;
        }
        binary_.back() &= kFirstWordMask;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>::BigInt(std::string_view str) noexcept {
        if (str.size() == 0) {
            std::cerr << "Empty string" << std::endl;
            std::terminate();
        }

        if (str[0] == '-') {
            is_positive_ = false;
            str.remove_prefix(1);
        }

        if (str.size() >= 2 && str[0] == '0' && str[1] == 'b') {
            str.remove_prefix(2);
            auto set_bit = [this](size_t bit_idx) {
                binary_[kWordCount - 1 - (bit_idx / 32)] &= 1u << (bit_idx % 32);
            };
            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '1') {
                    set_bit(i);
                }
            }
            *this >>= bit_size - str.size();
            binary_.back() &= kFirstWordMask;
        } else {
            std::cerr << "Only bytes are allowed" << std::endl;
            std::terminate();
        }
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator<<=(size_t shift) noexcept {
        if (shift >= bit_size) {
            shift %= bit_size;
        }
        if (shift == 0) {
            return *this;
        }
        size_t word_offset = shift / 32;
        size_t bit_offset = shift % 32;
        auto get_shifted_word = [&](size_t idx) -> uint32_t {
            uint32_t ret = 0;
            if (idx >= word_offset) {
                ret |= binary_[idx - word_offset] << bit_offset;
                if (bit_offset != 0 && idx > word_offset) [[likely]] {
                    ret |= binary_[idx - word_offset - 1] >> (32 - bit_offset);
                }
            }
            return ret;
        };

        for (size_t i = 0; i < kWordCount; ++i) {
            size_t idx = kWordCount - 1 - i;
            binary_[idx] = get_shifted_word(idx);
        }
        binary_.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator>>=(size_t shift) noexcept {
        if (shift >= bit_size) {
            shift %= bit_size;
        }
        if (shift == 0) {
            return *this;
        }
        size_t word_offset = shift / 32;
        size_t bit_offset = shift % 32;
        auto get_shifted_word = [&](size_t idx) -> uint32_t {
            uint32_t ret = 0;
            if (size_t shifted_idx = idx + word_offset; shifted_idx < kWordCount) {
                ret |= binary_[shifted_idx] >> bit_offset;
                if (bit_offset != 0 && shifted_idx + 1 < kWordCount) [[likely]] {
                    ret |= binary_[shifted_idx + 1] << (32 - bit_offset);
                }
            }
            return ret;
        };

        for (size_t i = 0; i < kWordCount; ++i) {
            binary_[i] = get_shifted_word(i);
        }
        binary_.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::operator<<(size_t shift_int) const noexcept {
        return BigInt{*this} <<= shift_int;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::operator>>(size_t shift_int) const noexcept {
        return BigInt{*this} >>= shift_int;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::operator-() const noexcept {
        BigInt copy = *this;
        copy.is_positive_ ^= true;
        return copy;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator+=(const BigInt& rhs) noexcept {
        if (is_positive_ ^ rhs.is_positive_) {
            return *this -= -rhs;
        }

        const uint32_t first_bit_mask = 1u << 31;
        uint32_t carry = 0;
        for (size_t i = 0; i < kWordCount; ++i) {
            if (kMaxInt - carry < rhs.binary_[i]) {
                continue;
            }

            uint32_t tmp = rhs.binary_[i] + carry;
            if (kMaxInt - binary_[i] < tmp) {
                if (binary_[i] >= first_bit_mask) {
                    binary_[i] ^= first_bit_mask;
                } else {
                    tmp ^= first_bit_mask;
                }
                binary_[i] += tmp;
                binary_[i] ^= first_bit_mask;
                carry = 1;
            } else {
                binary_[i] += tmp;
                carry = 0;
            }
        }
        binary_.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator-=(const BigInt& rhs) noexcept {
        if (is_positive_ ^ rhs.is_positive_) {
            return *this += -rhs;
        }

        if (auto cmp = *this <=> rhs; cmp == 0) {
            binary_ = {};
            return *this;
        } else if ((cmp > 0 && !is_positive_) || (cmp < 0 && is_positive_)) {
            BigInt tmp = *this;
            *this = rhs;
            *this -= tmp;
            is_positive_ ^= true;
            return *this;
        }

        uint32_t borrow = 0;
        for (size_t i = 0; i < kWordCount; ++i) {
            if (kMaxInt - borrow < rhs.binary_[i]) {
                continue;
            }

            uint32_t tmp = rhs.binary_[i] + borrow;
            if (binary_[i] < tmp) {
                binary_[i] += kMaxInt - tmp + 1;
                borrow = 1;
            } else {
                binary_[i] -= tmp;
                borrow = 0;
            }
        }
        binary_.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator*=(const BigInt& rhs) noexcept {
        if (rhs == 0) {
            binary_ = {};
            return *this;
        }

        is_positive_ ^= !rhs.is_positive_;
        const uint32_t first_bit_mask = 1u << 31;

        // https://www.geeksforgeeks.org/karatsuba-algorithm-for-fast-multiplication-using-divide-and-conquer-algorithm/
        BigInt ret;
        for (size_t i = 0; i < kWordCount; ++i) {
            BigInt tmp;
            for (size_t j = 0; j < kWordCount; ++j) {
                size_t idx = i + j;
                if (idx >= kWordCount) {
                    break;
                }
                
                uint64_t prod = static_cast<uint64_t>(binary_[i]) * rhs.binary_[j];
                uint32_t prod_l = static_cast<uint32_t>(prod & kMaxInt);
                uint32_t prod_h = static_cast<uint32_t>(prod >> 32);

                if (idx + 1 < kWordCount) {
                    tmp.binary_[idx + 1] = prod_h;
                }

                if (kMaxInt - prod_l < tmp.binary_[idx]) {
                    if (prod_l > tmp.binary_[idx]) {
                        prod_l ^= first_bit_mask;
                    } else {
                        tmp.binary_[idx] ^= first_bit_mask;
                    }
                    tmp.binary_[idx] += prod_l;
                    tmp.binary_[idx] ^= first_bit_mask;

                    if (idx + 1 < kWordCount) {
                        tmp.binary_[idx + 1] += 1;
                    }
                } else {
                    tmp.binary_[idx] += prod_l;
                }
            }
            ret += tmp;
        }

        *this = ret;
        binary_.at(0) &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr std::pair<BigInt<bit_size>, BigInt<bit_size>> BigInt<bit_size>::DivideUnsigned(const BigInt& rhs) const noexcept {
        if (rhs == 0) {
            std::cerr << "Dividing by 0" << std::endl;
            std::terminate();
        } else if (*this == 0) {
            return {0, 0};
        } else if (rhs == 1) {
            return {*this, 0};
        } else if (rhs == 2) {
            return {*this >> 1, binary_[0] & 1};
        } else if (*this == rhs) {
            return {1, 0};
        }

        // fast division for powers of 2
        if ((rhs & (rhs - 1)) == 0) {
            for (size_t i = 0; i < kWordCount; ++i) {
                if (rhs.binary_[i] != 0) {
                    size_t bit_idx = std::bit_width(rhs.binary_[i]) + i * 32;
                    return {*this >> (bit_idx - 1), *this & (rhs - 1)};
                }
            }
        }

        // use binary search to find quotient
        auto mul_greater = [](const BigInt& lhs, const BigInt& rhs, const BigInt& cmp) -> bool {
            // multiply numbers, until it is clear that cmp is greater than their product

            return false;
        };

        BigInt l = 1;
        BigInt r = *this;

        while (l < r - 1) {
            BigInt m = (l + r) / 2; // shouldn't overflow
            if (mul_greater(m, rhs, *this)) {
                r = m - 1;
            } else {
                l = m;
            }
        }

        return {l, *this - l * rhs};
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator/=(const BigInt& rhs) noexcept {
        is_positive_ ^= !rhs.is_positive_;
        *this = DivideUnsigned(rhs).first;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator%=(const BigInt& rhs) noexcept {
        if (rhs == 1) {
            std::cerr << "Trying to take a remainder of dividing by 1" << std::endl;
            std::terminate();
        }
        *this = DivideUnsigned(rhs).second;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::operator~() const noexcept {
        BigInt ret;
        ret.is_positive_ = is_positive_;
        for (size_t i = 0; i < kWordCount; ++i) {
            ret.binary_[i] = ~binary_[i];
        }
        ret.back() &= kFirstWordMask;
        return ret;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator^=(const BigInt& other) noexcept {
        for (size_t i = 0; i < kWordCount; ++i) {
            binary_[i] ^= other.binary_[i];
        }
        binary_.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator|=(const BigInt& other) noexcept {
        for (size_t i = 0; i < kWordCount; ++i) {
            binary_[i] |= other.binary_[i];
        }
        binary_.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator&=(const BigInt& other) noexcept {
        for (size_t i = 0; i < kWordCount; ++i) {
            binary_[i] &= other.binary_[i];
        }
        binary_.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::Power(size_t exp) const noexcept {
        if (exp == 0) {
            return 1;
        } else if (exp == 1) {
            return *this;
        }

        BigInt<bit_size> tmp = Power(exp / 2);
        if (exp % 2 == 0) {
            return tmp * tmp;
        } else {
            return tmp * tmp * *this;
        }
    }

    template<size_t bit_size>
    constexpr bool BigInt<bit_size>::operator==(const BigInt& rhs) const noexcept {
        return (*this <=> rhs) == 0;
    }

    template<size_t bit_size>
    constexpr auto BigInt<bit_size>::operator<=>(const BigInt& rhs) const noexcept {
        for (size_t i = 0; i < kWordCount; ++i) {
            size_t idx = kWordCount - 1 - i;
            if (binary_.at(idx) != rhs.binary_.at(idx)) {
                if (is_positive_) {
                    return binary_.at(idx) <=> rhs.binary_.at(idx);
                } else {
                    return rhs.binary_.at(idx) <=> binary_.at(idx);
                }
            }
        }

        return std::strong_ordering::equal;
    }

    // Arithmetic opeartors
    template<size_t bit_size>
    constexpr BigInt<bit_size> operator+(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} += rhs;;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator+(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} += BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator+(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} += rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator-(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} -= rhs;;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator-(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} -= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator-(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} -= rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator*(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} *= rhs;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator*(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} *= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator*(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} *= rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator/(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} /= rhs;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator/(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} /= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator/(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} /= rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator%(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} %= rhs;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator%(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} %= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator%(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} %= rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator&(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} &= rhs;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator&(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} &= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator&(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} &= rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator|(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} |= rhs;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator|(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} |= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator|(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} |= rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator^(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} ^= rhs;
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator^(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} ^= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<bit_size> operator^(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} ^= rhs;
    }

} // namespace algo
