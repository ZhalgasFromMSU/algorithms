#pragma once

#include <compare>
#include <iostream>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <array>
#include <bit>

namespace algo {

    template<size_t bit_size>
    class BigInt;

    template<typename T>
    struct IsBigInt : std::false_type {};

    template<size_t bit_size>
    struct IsBigInt<BigInt<bit_size>> : std::true_type {};

    template<size_t bit_capacity>
    class BigInt {
        static_assert(bit_capacity > 0);

        static constexpr uint32_t kMaxWord = std::numeric_limits<uint32_t>::max();
        static constexpr size_t kWordCapacity = (bit_capacity + 31) / 32;
        static constexpr uint32_t kFirstWordMask = bit_capacity % 32 == 0 ? kMaxWord : (1u << bit_capacity % 32) - 1u;

    public:
        constexpr BigInt() noexcept = default;
        constexpr BigInt(int64_t integer) noexcept;
        constexpr BigInt(std::string_view str) noexcept;

        template<typename It>
            requires( *It{} -> uint32_t )
        constexpr BigInt(It begin, It end, bool is_positive = true) noexcept;

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

        constexpr size_t BitWidth() const noexcept;

        constexpr void PrintBinary() const noexcept;

        bool is_positive = true;
        std::array<uint32_t, kWordCapacity> binary = {}; // number is storred right to left, e.g. most significant bits are at the end of an array
                                                      // can't use bitset here, because not constexp (since C++23)
        size_t words_count = 0;

    private:
        constexpr bool TrySetWordsCount(size_t val) noexcept;

        constexpr BigInt& MultiplyUnsigned(const BigInt&) noexcept;

        constexpr std::pair<BigInt, BigInt> DivideUnsigned(const BigInt&) const noexcept; // returns div and remainder

    };

    // impl
    template<size_t bit_size>
    constexpr BigInt<bit_size>::BigInt(int64_t from) noexcept {
        if (from < 0) {
            is_positive = false;
            from = -from;
        }

        binary[0] = from & ((1ull << 32) - 1);
        TrySetWordsCount(1);

        if (kWordCapacity > 1) {
            binary[1] = from >> 32;
            TrySetWordsCount(2);
        }
        binary.back() &= kFirstWordMask;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>::BigInt(std::string_view str) noexcept {
        if (str.size() == 0) {
            std::cerr << "Empty string" << std::endl;
            std::terminate();
        }

        if (str[0] == '-') {
            is_positive = false;
            str.remove_prefix(1);
        }

        if (str.size() >= 2 && str[0] == '0' && str[1] == 'b') {
            str.remove_prefix(2);
            auto set_bit = [this](size_t bit_idx) {
                binary[bit_idx / 32] |= 1u << (bit_idx % 32);
            };
            size_t count = 0;
            for (size_t i = 0; i < str.size(); ++i) {
                size_t idx = str.size() - 1 - i;
                if (str[idx] == '1') {
                    if (count > bit_size - 1) {
                        std::cerr << "String is too long" << std::endl;
                        std::terminate();
                    }
                    set_bit(count);
                    TrySetWordsCount((count + 31) / 32);
                }
                if (str[i] != '\'') {
                    count += 1;
                }
            }

            binary.back() &= kFirstWordMask;
        } else {
            std::cerr << "Only bytes are allowed" << std::endl;
            std::terminate();
        }
    }

    template<size_t bit_size>
    template<typename It>
        requires ( *It{} -> uint32_t )
    constexpr BigInt<bit_size>::BigInt(It begin, It end, bool is_positive) noexcept
        : is_positive{is_positive}
    {
        size_t counter = 1;
        for (It it = begin; it != end; ++it, ++counter) {
            if (*it > 0) {
                words_count = counter;
            }
        }
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator<<=(size_t shift) noexcept {
        if (shift >= bit_size) {
            std::cerr << "Shifting by more than size" << std::endl;
            std::terminate();
        }
        if (shift == 0) {
            return *this;
        }
        size_t word_offset = shift / 32;
        size_t bit_offset = shift % 32;
        auto get_shifted_word = [&](size_t idx) -> uint32_t {
            uint32_t ret = 0;
            if (idx >= word_offset) {
                ret |= binary[idx - word_offset] << bit_offset;
                if (bit_offset != 0 && idx > word_offset) [[likely]] {
                    ret |= binary[idx - word_offset - 1] >> (32 - bit_offset);
                }
            }
            return ret;
        };

        bool set = false;
        for (size_t i = 0; i < kWordCapacity; ++i) {
            size_t idx = kWordCapacity - 1 - i;
            binary[idx] = get_shifted_word(idx);
            if (!set && TrySetWordsCount(idx + 1)) {
                set = true;
            }
        }
        binary.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator>>=(size_t shift) noexcept {
        if (shift >= bit_size) {
            std::cerr << "Shifting by more than size" << std::endl;
            std::terminate();
        }
        if (shift == 0) {
            return *this;
        }
        size_t word_offset = shift / 32;
        size_t bit_offset = shift % 32;
        auto get_shifted_word = [&](size_t idx) -> uint32_t {
            uint32_t ret = 0;
            if (size_t shifted_idx = idx + word_offset; shifted_idx < kWordCapacity) {
                ret |= binary[shifted_idx] >> bit_offset;
                if (bit_offset != 0 && shifted_idx + 1 < kWordCapacity) [[likely]] {
                    ret |= binary[shifted_idx + 1] << (32 - bit_offset);
                }
            }
            return ret;
        };

        const size_t iterations = words_count;
        for (size_t i = 0; i < iterations; ++i) {
            binary[i] = get_shifted_word(i);
            TrySetWordsCount(i + 1);
        }
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
        copy.is_positive ^= true;
        return copy;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator+=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            return *this -= -rhs;
        }

        const size_t iterations = std::max(words_count, rhs.words_count);
        const uint32_t first_bit_mask = 1u << 31;
        uint32_t carry = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if (kMaxWord - carry < rhs.binary[i]) {
                continue;
            }

            uint32_t tmp = rhs.binary[i] + carry;
            if (kMaxWord - binary[i] < tmp) {
                if (binary[i] >= first_bit_mask) {
                    binary[i] ^= first_bit_mask;
                } else {
                    tmp ^= first_bit_mask;
                }
                binary[i] += tmp;
                binary[i] ^= first_bit_mask;
                carry = 1;
            } else {
                binary[i] += tmp;
                carry = 0;
            }
        }
        if (iterations < kWordCapacity && carry) {
            binary[iterations] = carry;
            words_count = iterations + 1;
        } else {
            words_count = iterations;
        }

        binary.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator-=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            return *this += -rhs;
        }

        if (auto cmp = *this <=> rhs; cmp == 0) {
            binary = {};
            words_count = 0;
            return *this;
        } else if ((cmp > 0 && !is_positive) || (cmp < 0 && is_positive)) {
            BigInt tmp = *this;
            *this = rhs;
            *this -= tmp;
            is_positive ^= true;
            return *this;
        }

        uint32_t borrow = 0;
        for (size_t i = 0; i < words_count; ++i) {
            if (kMaxWord - borrow < rhs.binary[i]) {
                continue;
            }

            uint32_t tmp = rhs.binary[i] + borrow;
            if (binary[i] < tmp) {
                binary[i] += kMaxWord - tmp + 1;
                borrow = 1;
            } else {
                binary[i] -= tmp;
                borrow = 0;
            }

            if (binary[i]) {
                words_count = i + 1;
            }
        }
        binary.back() &= kFirstWordMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr bool BigInt<bit_size>::TrySetWordsCount(size_t val) noexcept {
        if (val == 0) {
            words_count = 0;
        }
        if (binary[val - 1] == 0) {
            return false;
        }
        if (val == kWordCapacity && (binary.back() & kFirstWordMask) == 0) {
            return false;
        }
        words_count = val;
        return true;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::MultiplyUnsigned(const BigInt& rhs) noexcept {
        if (rhs.words_count == 0 || words_count == 0) {
            binary = {};
            words_count = 0;
            return *this;
        }

        is_positive ^= !rhs.is_positive;
        if (rhs == 1) {
            return *this;
        } else if (*this == 1) {
            std::copy(rhs.binary.begin(), rhs.binary.begin() + rhs.words_count, binary.begin());
            words_count = rhs.words_count;
            return *this;
        }

        // Fast multiplication for powers of 2
        if ((rhs & (rhs - 1)) == 0) {
            return *this <<= rhs.BitWidth() - 1;
        }

        constexpr uint32_t first_bit_mask = 1u << 31;
        const size_t bit_width = BitWidth();
        const size_t rhs_bit_width = rhs.BitWidth();

        // Case when multiplication can be done linearly, without extra stack allocation for temporary result
        if (words_count == 1 || rhs.words_count == 1) {
            uint32_t tmp_word = binary[0];
            binary[0] = 0;
            const size_t iterations = std::max(words_count, rhs.words_count);
            for (size_t i = 0; i < iterations; ++i) {
                uint64_t prod;
                if (words_count == 1) {
                    prod = static_cast<uint64_t>(tmp_word) * rhs.binary[i];
                } else {
                    prod = static_cast<uint64_t>(tmp_word) * rhs.binary[0];
                    if (i + 1 < kWordCapacity) {
                        tmp_word = binary[i + 1];
                    }
                }
                uint32_t prod_l = static_cast<uint32_t>(prod & kMaxWord);
                uint32_t prod_h = static_cast<uint32_t>(prod >> 32);

                if (i + 1 < kWordCapacity) {
                    binary[i + 1] = prod_h;
                }

                if (kMaxWord - prod_l < binary[i]) {
                    if (prod_l > binary[i]) {
                        prod_l ^= first_bit_mask;
                    } else {
                        binary[i] ^= first_bit_mask;
                    }
                    binary[i] += prod_l;
                    binary[i] ^= first_bit_mask;

                    if (i + 1 < kWordCapacity) {
                        binary[i + 1] += 1;
                    }
                } else {
                    binary[i] += prod_l;
                }
            }

            if (iterations + 1 < kWordCapacity && binary[iterations + 1]) {
                words_count = iterations + 2;
            } else {
                words_count = iterations + 1;
            }

            if (!(binary.back() &= kFirstWordMask) && words_count == kWordCapacity) {
                words_count -= 1;
            }

            return *this;
        }

        if constexpr (bit_size <= 1024) { // for small numbers do quadratic multiplication
            BigInt ret;
            for (size_t i = 0; i < kWordCapacity; ++i) {
                BigInt tmp;
                for (size_t j = 0; j < kWordCapacity; ++j) {
                    size_t idx = i + j;
                    if (idx >= kWordCapacity) {
                        break;
                    }
                
                    uint64_t prod = static_cast<uint64_t>(binary[i]) * rhs.binary[j];
                    uint32_t prod_l = static_cast<uint32_t>(prod & kMaxWord);
                    uint32_t prod_h = static_cast<uint32_t>(prod >> 32);

                    if (idx + 1 < kWordCapacity) {
                        tmp.binary[idx + 1] = prod_h;
                    }

                    if (kMaxWord - prod_l < tmp.binary[idx]) {
                        if (prod_l > tmp.binary[idx]) {
                            prod_l ^= first_bit_mask;
                        } else {
                            tmp.binary[idx] ^= first_bit_mask;
                        }
                        tmp.binary[idx] += prod_l;
                        tmp.binary[idx] ^= first_bit_mask;

                        if (idx + 1 < kWordCapacity) {
                            tmp.binary[idx + 1] += 1;
                        }
                    } else {
                        tmp.binary[idx] += prod_l;
                    }
                }
                ret += tmp;
            }

            *this = ret;
            binary.back() &= kFirstWordMask;
            return *this;
        } else {
            // https://www.geeksforgeeks.org/karatsuba-algorithm-for-fast-multiplication-using-divide-and-conquer-algorithm/
            size_t shift = std::max(bit_width, rhs_bit_width) / 2;
            BigInt bottom_half = (BigInt{1} << shift) - 1;

            BigInt this_l = *this & bottom_half;
            BigInt this_h = *this >> shift;

            BigInt rhs_l = rhs & bottom_half;
            BigInt rhs_h = rhs >> shift;

            BigInt h_h = this_h * rhs_h;
            BigInt l_l = this_l * rhs_l;
            BigInt mix = (this_h + this_l) * (rhs_h + rhs_l);

            *this = (((h_h << shift) + mix - l_l - h_h) << shift) + l_l;
            binary.back() &= kFirstWordMask;
            return *this;
        }
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator*=(const BigInt& rhs) noexcept {
        return MultiplyUnsigned(rhs);
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
            return {*this >> 1, binary[0] & 1};
        } else if (*this == rhs) {
            return {1, 0};
        }

        // fast division for powers of 2
        if ((rhs & (rhs - 1)) == 0) {
            return {*this >> (rhs.BitWidth() - 1), *this & (rhs - 1)};
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
        is_positive ^= !rhs.is_positive;
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
        ret.is_positive = is_positive;
        for (size_t i = 0; i < kWordCapacity; ++i) {
            ret.binary[i] = ~binary[i];
            if (ret.binary[i]) {
                ret.words_count = i + 1;
            }
        }

        if (ret.binary.back() && !(ret.binary.back() &= kFirstWordMask)) {
            ret.words_count -= 1;
        }
        return ret;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator^=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        for (size_t i = 0; i < iterations; ++i) {
            binary[i] ^= other.binary[i];
            if (binary[i]) {
                words_count = i + 1;
            }
        }
        if (binary.back() && !(binary.back() &= kFirstWordMask)) {
            words_count -= 1;
        }
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator|=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        for (size_t i = 0; i < iterations; ++i) {
            binary[i] |= other.binary[i];
            if (binary[i]) {
                words_count = i + 1;
            }
        }
        if (binary.back() && !(binary.back() &= kFirstWordMask)) {
            words_count -= 1;
        }
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator&=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        for (size_t i = 0; i < iterations; ++i) {
            binary[i] &= other.binary[i];
            if (binary[i]) {
                words_count = i + 1;
            }
        }
        if (binary.back() && !(binary.back() &= kFirstWordMask)) {
            words_count -= 1;
        }
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
    constexpr size_t BigInt<bit_size>::BitWidth() const noexcept {
        if (words_count == 0) {
            return 0;
        }

        return words_count * 32 + std::bit_width(binary[words_count - 1]);
    }

    template<size_t bit_size>
    constexpr void BigInt<bit_size>::PrintBinary() const noexcept {
        std::cerr << words_count << " ";
        if (is_positive) {
            std::cerr << "+ ";
        } else {
            std::cerr << "- ";
        }
        for (size_t i = 0; i < kWordCapacity; ++i) {
            uint32_t n = 1u << 31;
            while (n != 0) {
                std::cerr << (binary[kWordCapacity - 1 - i] & n ? 1 : 0);
                n >>= 1;
            }
        }
        std::cerr << std::endl;
    }

    template<size_t bit_size>
    constexpr bool BigInt<bit_size>::operator==(const BigInt& rhs) const noexcept {
        return (*this <=> rhs) == 0;
    }

    template<size_t bit_size>
    constexpr auto BigInt<bit_size>::operator<=>(const BigInt& rhs) const noexcept {
        if (words_count == 0 && rhs.words_count == 0) {
            return std::strong_ordering::equal;
        } else if (words_count == 0) {
            return rhs.is_positive ? std::strong_ordering::less : std::strong_ordering::greater;
        } else if (rhs.words_count == 0) {
            return is_positive ? std::strong_ordering::greater : std::strong_ordering::less;
        }

        if (is_positive ^ rhs.is_positive) {
            return is_positive <=> rhs.is_positive;
        }

        if (words_count != rhs.words_count) {
            if (is_positive) {
                return words_count <=> rhs.words_count;
            } else {
                return rhs.words_count <=> words_count;
            }
        }

        for (size_t i = 0; i < kWordCapacity; ++i) {
            size_t idx = kWordCapacity - 1 - i;
            if (binary.at(idx) != rhs.binary.at(idx)) {
                if (is_positive) {
                    return binary.at(idx) <=> rhs.binary.at(idx);
                } else {
                    return rhs.binary.at(idx) <=> binary.at(idx);
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
