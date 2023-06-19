#pragma once

#include <compare>
#include <iostream>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <array>
#include <bit>

namespace algo {

    template<size_t word_capacity>
    class BigInt;

    template<typename T>
    struct IsBigInt : std::false_type {};

    template<size_t word_capacity>
    struct IsBigInt<BigInt<word_capacity>> : std::true_type {
        static constexpr size_t kWordCapacity = word_capacity;
    };

    template<size_t word_capacity>
    class BigInt {
        static_assert(word_capacity > 0);

        static constexpr uint32_t kMaxWord = std::numeric_limits<uint32_t>::max();

    public:
        constexpr BigInt() noexcept = default;
        constexpr BigInt(int64_t integer) noexcept;
        constexpr BigInt(std::string_view str) noexcept;

        template<typename It>
            requires( std::is_same_v<std::remove_cvref_t<decltype(*It{})>, uint32_t> )
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
        constexpr const uint32_t* cbegin() const noexcept;
        constexpr const uint32_t* cend() const noexcept;

        constexpr void PrintBinary() const noexcept;
        constexpr void PrintWords() const noexcept;

        bool is_positive = true;
        std::array<uint32_t, word_capacity> binary = {}; // number is storred right to left, e.g. most significant bits are at the end of an array
                                                         // can't use bitset here, because not constexp (since C++23)
        size_t words_count = 0;

    private:
        template<typename It>
            requires( std::is_same_v<std::remove_cvref_t<decltype(*It{})>, uint32_t> )
        constexpr BigInt& UnsignedAddRange(It begin, It end) noexcept;

        template<typename It>
            requires( std::is_same_v<std::remove_cvref_t<decltype(*It{})>, uint32_t> )
        constexpr BigInt& UnsignedSubSmallerRange(It begin, It end) noexcept;

        constexpr BigInt& ShortMultiplyBy(const BigInt&) noexcept; // either this or rhs is lesser than 32 bits
        constexpr BigInt& UnsignedMultiplyBy(const BigInt&) noexcept;

        template<size_t small_word_capacity>
        constexpr BigInt& KaratsubaMultiply(const BigInt&) noexcept;

        constexpr BigInt ShortUnsignedDivideBy(uint32_t) noexcept; // returns remainder
        constexpr BigInt UnsignedDivideBy(const BigInt&) noexcept; // returns remainder

    };

    // impl
    template<size_t word_capacity>
    constexpr BigInt<word_capacity>::BigInt(int64_t from) noexcept {
        if (from < 0) {
            is_positive = false;
            from = -from;
        }

        if ((binary[0] = from & ((1ull << 32) - 1))) {
            words_count = 1;
        }

        if (word_capacity > 1) {
            if ((binary[1] = from >> 32)) {
                words_count = 2;
            }
        }
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>::BigInt(std::string_view str) noexcept {
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
                words_count = bit_idx / 32 + 1;
            };
            size_t count = 0;
            for (size_t i = 0; i < str.size(); ++i) {
                size_t idx = str.size() - 1 - i;
                if (str[idx] == '1') {
                    set_bit(count);
                }
                if (str[idx] != '\'') {
                    count += 1;
                    if (count >= word_capacity * 32) {
                        std::cerr << "String is too long" << std::endl;
                        std::terminate();
                    }
                }
            }
        } else {
            std::cerr << "Only bytes are allowed" << std::endl;
            std::terminate();
        }
    }

    template<size_t word_capacity>
    template<typename It>
        requires( std::is_same_v<std::remove_cvref_t<decltype(*It{})>, uint32_t> )
    constexpr BigInt<word_capacity>::BigInt(It begin, It end, bool is_positive) noexcept
        : is_positive{is_positive}
    {
        size_t counter = 1;
        auto set = binary.begin();
        for (It it = begin; it != end; ++it, ++set, ++counter) {
            if (counter > word_capacity) {
                std::cerr << "Range is too big" << std::endl;
                std::terminate();
            }

            if ((*set = *it)) {
                words_count = counter;
            }
        }
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator<<=(size_t shift) noexcept {
        if (shift >= word_capacity * 32) {
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

        words_count = 0;
        for (size_t i = 0; i < word_capacity; ++i) {
            size_t idx = word_capacity - 1 - i;
            if ((binary[idx] = get_shifted_word(idx)) && words_count == 0) {
                words_count = idx + 1;
            }
        }
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator>>=(size_t shift) noexcept {
        if (shift >= word_capacity * 32) {
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
            if (size_t shifted_idx = idx + word_offset; shifted_idx < word_capacity) {
                ret |= binary[shifted_idx] >> bit_offset;
                if (bit_offset != 0 && shifted_idx + 1 < word_capacity) [[likely]] {
                    ret |= binary[shifted_idx + 1] << (32 - bit_offset);
                }
            }
            return ret;
        };

        const size_t iterations = words_count;
        words_count = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if ((binary[i] = get_shifted_word(i))) {
                words_count = i + 1;
            }
        }
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> BigInt<word_capacity>::operator<<(size_t shift_int) const noexcept {
        return BigInt{*this} <<= shift_int;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> BigInt<word_capacity>::operator>>(size_t shift_int) const noexcept {
        return BigInt{*this} >>= shift_int;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> BigInt<word_capacity>::operator-() const noexcept {
        BigInt copy = *this;
        copy.is_positive ^= true;
        return copy;
    }

    template<size_t word_capacity>
    template<typename It>
        requires( std::is_same_v<std::remove_cvref_t<decltype(*It{})>, uint32_t> )
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::UnsignedAddRange(It begin, It end) noexcept {
        constexpr uint32_t first_bit_mask = 1u << 31;
        uint32_t carry = 0;

        size_t old_words_count = words_count;
        size_t i = 0;
        while (begin != end) {
            if (i >= word_capacity) {
                std::cerr << "Range is bigger than capacity" << std::endl;
                std::terminate();
            }
            if (kMaxWord - carry >= *begin) {
                uint32_t tmp = *begin + carry;
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
                    if ((binary[i] += tmp)) {
                        words_count = i + 1;
                    }
                    carry = 0;
                }
            }
            ++begin;
            ++i;
        }

        while (carry != 0 && i < word_capacity) {
            if (kMaxWord - carry < binary[i]) {
                binary[i] = 0;
            } else {
                binary[i] += carry;
                carry = 0;
                words_count = i + 1;
            }
            ++i;
        }

        if (carry != 0) {
            std::cerr << "Undefined behavior, addition" << std::endl;
            std::terminate();
        }

        words_count = std::max(old_words_count, words_count);

        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator+=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            return *this -= -rhs;
        }
        return UnsignedAddRange(rhs.binary.begin(), rhs.binary.begin() + rhs.words_count);
    }

    template<size_t word_capacity>
    template<typename It>
        requires( std::is_same_v<std::remove_cvref_t<decltype(*It{})>, uint32_t> )
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::UnsignedSubSmallerRange(It begin, It end) noexcept {
        size_t old_words_count = words_count;
        uint32_t borrow = 0;
        size_t i = 0;
        while (begin != end) {
            if (i >= old_words_count) {
                std::cerr << "Subtraction by greater value" << std::endl;
                std::terminate();
            }
            if (kMaxWord - borrow >= *begin) {
                uint32_t tmp = *begin + borrow;
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
            ++begin;
            ++i;
        }

        while (borrow && i < old_words_count) {
            if (borrow > binary[i]) {
                binary[i] = kMaxWord;
                words_count = i + 1;
            } else {
                if ((binary[i] -= borrow)) {
                    words_count = i + 1;
                }
                borrow = 0;
            }
            ++i;
        }

        if (borrow) {
            std::cerr << "Undefined behavior sub" << std::endl;
            std::terminate();
        }

        if (binary[old_words_count - 1]) {
            words_count = old_words_count;
        }
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator-=(const BigInt& rhs) noexcept {
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

        return UnsignedSubSmallerRange(rhs.binary.begin(), rhs.binary.begin() + rhs.words_count);
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::ShortMultiplyBy(const BigInt& rhs) noexcept {
        constexpr uint32_t first_bit_mask = 1u << 31;

        uint32_t tmp_word = binary[0];
        binary[0] = 0;
        const size_t iterations = std::max(words_count, rhs.words_count);
        for (size_t i = 0; i < iterations; ++i) {
            uint64_t prod;
            if (words_count == 1) {
                prod = static_cast<uint64_t>(tmp_word) * rhs.binary[i];
            } else {
                prod = static_cast<uint64_t>(tmp_word) * rhs.binary[0];
                if (i + 1 < word_capacity) {
                    tmp_word = binary[i + 1];
                }
            }
            uint32_t prod_l = static_cast<uint32_t>(prod & kMaxWord);
            uint32_t prod_h = static_cast<uint32_t>(prod >> 32);

            if (i + 1 < word_capacity) {
                binary[i + 1] = prod_h;
            } else if (prod_h) {
                std::cerr << "Overflow!" << std::endl;
                std::terminate();
            }

            if (kMaxWord - prod_l < binary[i]) {
                if (prod_l > binary[i]) {
                    prod_l ^= first_bit_mask;
                } else {
                    binary[i] ^= first_bit_mask;
                }
                binary[i] += prod_l;
                binary[i] ^= first_bit_mask;

                if (i + 1 < word_capacity) {
                    binary[i + 1] += 1;
                } else if (prod_h) {
                    std::cerr << "Multiplication overflow" << std::endl;
                    std::terminate();
                }
            } else {
                binary[i] += prod_l;
            }
        }

        if (iterations < word_capacity && binary[iterations]) {
            words_count = iterations + 1;
        } else {
            words_count = iterations;
        }

        return *this;
    }

    template<size_t word_capacity>
    template<size_t small_word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::KaratsubaMultiply(const BigInt& rhs) noexcept {
        size_t end_thr = std::max(words_count, rhs.words_count);
        size_t mid_thr = (end_thr + 1) / 2;
        BigInt<small_word_capacity> this_l {binary.begin(), binary.begin() + mid_thr};
        BigInt<small_word_capacity> this_h {binary.begin() + mid_thr, binary.begin() + end_thr};
        BigInt<small_word_capacity> rhs_l {rhs.binary.begin(), rhs.binary.begin() + mid_thr};
        BigInt<small_word_capacity> rhs_h {rhs.binary.begin() + mid_thr, rhs.binary.begin() + end_thr};

        BigInt<small_word_capacity> mix = (this_h + this_l) * (rhs_h + rhs_l);
        this_h *= rhs_h;
        this_l *= rhs_l;

        size_t shift = mid_thr * 32;
        *this = {this_h.binary.begin(), this_h.binary.begin() + this_h.words_count};
        *this <<= shift;
        UnsignedAddRange(mix.binary.begin(), mix.binary.begin() + mix.words_count);
        UnsignedSubSmallerRange(this_h.binary.begin(), this_h.binary.begin() + this_h.words_count);
        UnsignedSubSmallerRange(this_l.binary.begin(), this_l.binary.begin() + this_l.words_count);
        *this <<= shift;
        UnsignedAddRange(this_l.binary.begin(), this_l.binary.begin() + this_l.words_count);
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::UnsignedMultiplyBy(const BigInt& rhs) noexcept {
        // Fast multiplication for powers of 2
        if ((rhs & (rhs - 1)).words_count == 0) {
            return *this <<= rhs.BitWidth() - 1;
        }

        // Case when multiplication can be done linearly, without extra stack allocation for temporary result
        if (words_count == 1 || rhs.words_count == 1) {
            return ShortMultiplyBy(rhs);
        }

        if constexpr (word_capacity <= 40) { // for small numbers do quadratic multiplication
            BigInt ret;
            for (size_t i = 0; i < words_count; ++i) {
                BigInt tmp = static_cast<int64_t>(binary[i]);
                tmp.ShortMultiplyBy(rhs);
                tmp <<= i * 32;
                ret += tmp;
            }

            return *this = ret;
        } else {
            // https://www.geeksforgeeks.org/karatsuba-algorithm-for-fast-multiplication-using-divide-and-conquer-algorithm/
            if (size_t max_size = words_count + rhs.words_count; max_size <= word_capacity / 16) {
                return KaratsubaMultiply<word_capacity / 16 + 1>(rhs);
            } else if (max_size <= word_capacity / 8) {
                return KaratsubaMultiply<word_capacity / 8 + 1>(rhs);
            } else if (max_size <= word_capacity / 4) {
                return KaratsubaMultiply<word_capacity / 4 + 1>(rhs);
            } else if (max_size <= word_capacity / 2) {
                return KaratsubaMultiply<word_capacity / 2 + 1>(rhs);
            }
            return KaratsubaMultiply<word_capacity>(rhs);
        }
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator*=(const BigInt& rhs) noexcept {
        if (rhs.words_count == 0 || words_count == 0) {
            binary = {};
            words_count = 0;
            return *this;
        }

        bool sign = is_positive ^ !rhs.is_positive;

        if (rhs.words_count == 1 && rhs.binary[0] == 1) {
            return *this;
        } else if (words_count == 1 && binary[0] == 1) {
            std::copy(rhs.binary.begin(), rhs.binary.begin() + rhs.words_count, binary.begin());
            words_count = rhs.words_count;
            return *this;
        }

        UnsignedMultiplyBy(rhs);
        is_positive = sign;
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> BigInt<word_capacity>::ShortUnsignedDivideBy(uint32_t rhs) noexcept {
        BigInt ret;
        bool set = false;
        const size_t iterations = words_count;
        uint64_t window = 0;
        for (size_t i = 0; i < iterations; ++i) {
            window = (window << 32) + binary[iterations - 1 - i];
            if (window >= rhs) {
                binary[iterations - 1 - i] = window / rhs;
                if (!set && binary[iterations - 1 - i]) {
                    set = true;
                    words_count = iterations - i;
                }
                window %= rhs;
            } else {
                binary[iterations - 1 - i] = 0;
            }
        }

        if ((ret.binary[0] = static_cast<uint32_t>(window))) {
            ret.words_count = 1;
        }
        return ret;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> BigInt<word_capacity>::UnsignedDivideBy(const BigInt& rhs) noexcept {
        if (rhs.words_count == 0) {
            std::cerr << "Dividing by 0" << std::endl;
            std::terminate();
        } else if (words_count == 0) {
            return 0;
        } else if (rhs.words_count == 1 && rhs.binary[0] == 1) {
            return 0;
        } else if (rhs.words_count == 1 && rhs.binary[0] == 2) {
            BigInt remainder = binary[0] & 1;
            *this >>= 1;
            return remainder;
        } else if (words_count < rhs.words_count) {
            BigInt remainder = *this;
            *this = 0;
            return remainder;
        }

        is_positive = rhs.is_positive;

        // fast division for powers of 2
        if ((rhs & (rhs - 1)) == 0) {
            BigInt remainder = *this & (rhs - 1);
            *this >>= rhs.BitWidth() - 1;
            return remainder;
        }

        if (rhs.words_count == 1) {
            return ShortUnsignedDivideBy(rhs.binary[0]);
        }

        auto unsigned_cmp = []<size_t cap>(const BigInt<cap>& lhs, const BigInt<cap>& rhs) {
            if (auto cmp = lhs.words_count <=> rhs.words_count; cmp != 0) {
                return cmp;
            }

            for (size_t i = 0; i < lhs.words_count; ++i) {
                size_t idx = lhs.words_count - 1 - i;
                if (auto cmp = lhs.binary[idx] <=> rhs.binary[idx]; cmp != 0) {
                    return cmp;
                }
            }
            return std::strong_ordering::equal;
        };

        if (words_count - rhs.words_count <= 1) {
            // binary search
            int64_t l = 0;
            int64_t r = kMaxWord;
            while (l != r) {
                int64_t m = (l + r) / 2;
                BigInt tmp = *this;
                tmp.ShortUnsignedDivideBy(m);

                if (auto cmp = unsigned_cmp(tmp, rhs); cmp == 0) {
                    *this = m;
                    return 0;
                } else if (cmp < 0) {
                    r = m - 1;
                } else {
                    BigInt rmndr = *this - rhs * m;
                    if (auto cmp = unsigned_cmp(rmndr, rhs); cmp < 0) {
                        *this = m;
                        return rmndr;
                    }
                    l = m + 1;
                }
            }
            auto remainder = *this - rhs * l;
            *this = l;
            return remainder;
        }

        BigInt quotient;
        BigInt remainder = binary[words_count - 1];

        const size_t iterations = words_count;
        for (size_t i = 0; i < iterations; ++i) {
            size_t idx = iterations - 1 - i;
            if (unsigned_cmp(remainder, rhs) >= 0) {
                BigInt tmp = remainder;
                if (tmp.words_count - rhs.words_count > 1) {
                    std::cerr << "Temporary divisor and divident should differ less" << std::endl;
                    std::terminate();
                }
                remainder = tmp.UnsignedDivideBy(rhs);
                if (tmp.words_count > 1) {
                    std::cerr << "Quotient longer than 1 word" << std::endl;
                    std::terminate();
                }
                if ((quotient.binary[idx] = tmp.binary[0]) && quotient.words_count == 0) {
                    quotient.words_count = idx + 1;
                }
            } else {
                quotient.binary[idx] = 0;
            }
            if (idx > 0) {
                remainder <<= 32;
                remainder += binary[idx - 1];
            }
        }
        *this = quotient;
        return remainder;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator/=(const BigInt& rhs) noexcept {
        bool sign = is_positive ^ !rhs.is_positive;
        UnsignedDivideBy(rhs);
        is_positive = sign;
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator%=(const BigInt& rhs) noexcept {
        if (rhs == 1 || !rhs.is_positive) {
            std::cerr << "Trying to take a remainder of dividing by 1 or negative number" << std::endl;
            std::terminate();
        }
        bool sign = is_positive;
        *this = UnsignedDivideBy(rhs);
        is_positive = sign;
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> BigInt<word_capacity>::operator~() const noexcept {
        BigInt ret;
        ret.is_positive = is_positive;
        for (size_t i = 0; i < word_capacity; ++i) {
            if ((ret.binary[i] = ~binary[i])) {
                ret.words_count = i + 1;
            }
        }

        return ret;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator^=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if ((binary[i] ^= other.binary[i])) {
                words_count = i + 1;
            }
        }
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator|=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = iterations;
        for (size_t i = 0; i < iterations; ++i) {
            binary[i] |= other.binary[i];
        }
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity>& BigInt<word_capacity>::operator&=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if ((binary[i] &= other.binary[i])) {
                words_count = i + 1;
            }
        }
        return *this;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> BigInt<word_capacity>::Power(size_t exp) const noexcept {
        if (exp == 0) {
            return 1;
        } else if (exp == 1) {
            return *this;
        }

        BigInt<word_capacity> tmp = Power(exp / 2);
        if (exp % 2 == 0) {
            return tmp * tmp;
        } else {
            return tmp * tmp * *this;
        }
    }

    template<size_t word_capacity>
    constexpr size_t BigInt<word_capacity>::BitWidth() const noexcept {
        if (words_count == 0) {
            return 0;
        }

        return (words_count - 1) * 32 + std::bit_width(binary[words_count - 1]);
    }

    template<size_t word_capacity>
    constexpr const uint32_t* BigInt<word_capacity>::cbegin() const noexcept {
        return binary.begin();
    }

    template<size_t word_capacity>
    constexpr const uint32_t* BigInt<word_capacity>::cend() const noexcept {
        return binary.cbegin() + words_count;
    }

    template<size_t word_capacity>
    constexpr void BigInt<word_capacity>::PrintWords() const noexcept {
        std::cerr << words_count << " ";
        if (is_positive) {
            std::cerr << "+ ";
        } else {
            std::cerr << "- ";
        }
        for (size_t i = 0; i < word_capacity; ++i) {
            if (i != 0) {
                std::cerr << "'";
            }
            std::cerr << binary[word_capacity - 1 - i];
        }
        std::cerr << std::endl;
    }

    template<size_t word_capacity>
    constexpr void BigInt<word_capacity>::PrintBinary() const noexcept {
        std::cerr << words_count << " ";
        if (is_positive) {
            std::cerr << "+ ";
        } else {
            std::cerr << "- ";
        }
        for (size_t i = 0; i < word_capacity; ++i) {
            uint32_t n = 1u << 31;
            while (n != 0) {
                std::cerr << (binary[word_capacity - 1 - i] & n ? 1 : 0);
                n >>= 1;
            }
        }
        std::cerr << std::endl;
    }

    template<size_t word_capacity>
    constexpr bool BigInt<word_capacity>::operator==(const BigInt& rhs) const noexcept {
        return (*this <=> rhs) == 0;
    }

    template<size_t word_capacity>
    constexpr auto BigInt<word_capacity>::operator<=>(const BigInt& rhs) const noexcept {
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

        for (size_t i = 0; i < word_capacity; ++i) {
            size_t idx = word_capacity - 1 - i;
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
    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator+(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} += rhs;;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator+(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} += BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator+(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} += rhs;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator-(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} -= rhs;;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator-(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} -= BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator-(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} -= rhs;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator*(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} *= rhs;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator*(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} *= BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator*(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} *= rhs;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator/(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} /= rhs;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator/(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} /= BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator/(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} /= rhs;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator%(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} %= rhs;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator%(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} %= BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator%(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} %= rhs;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator&(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} &= rhs;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator&(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} &= BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator&(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} &= rhs;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator|(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} |= rhs;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator|(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} |= BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator|(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} |= rhs;
    }

    template<size_t word_capacity>
    constexpr BigInt<word_capacity> operator^(const BigInt<word_capacity>& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{lhs} ^= rhs;
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator^(const BigInt<word_capacity>& lhs, T&& rhs) noexcept {
        return BigInt<word_capacity>{lhs} ^= BigInt<word_capacity>{std::forward<T>(rhs)};
    }

    template<size_t word_capacity, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<word_capacity> operator^(T&& lhs, const BigInt<word_capacity>& rhs) noexcept {
        return BigInt<word_capacity>{std::forward<T>(lhs)} ^= rhs;
    }

} // namespace algo
