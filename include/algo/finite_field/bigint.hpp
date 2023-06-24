#pragma once

#include <compare>
#include <exception>
#include <iostream>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <array>
#include <bit>

namespace algo {

    template<size_t words_capacity>
    class BigInt;

    template<typename T>
    struct IsBigInt : std::false_type {};

    template<size_t words_capacity>
    struct IsBigInt<BigInt<words_capacity>> : std::true_type {
        static constexpr size_t kWordCapacity = words_capacity;
    };

    template<typename T>
    concept IntIter = std::is_same_v<std::remove_cvref_t<decltype(*T{})>, uint32_t>;

    template<size_t words_capacity>
    class BigInt {
        static_assert(words_capacity > 0);

        static constexpr uint32_t kMaxWord = std::numeric_limits<uint32_t>::max();

    public:
        constexpr BigInt() noexcept = default;
        constexpr BigInt(int64_t integer) noexcept;
        constexpr BigInt(std::string_view str) noexcept;

        template<IntIter It>
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

        constexpr BigInt Power(BigInt exp) const noexcept;
        constexpr BigInt Power(BigInt exp, const BigInt& mod) const noexcept;

        constexpr bool IsZero() const noexcept;
        constexpr bool IsOne() const noexcept;
        constexpr bool IsPowerOf2() const noexcept;
        constexpr size_t BitWidth() const noexcept;
        constexpr const uint32_t* cbegin() const noexcept;
        constexpr const uint32_t* cend() const noexcept;

        constexpr void PrintDecimal() const noexcept;
        constexpr void PrintBinary() const noexcept;
        constexpr void PrintWords() const noexcept;

        bool is_positive = true;
        std::array<uint32_t, words_capacity> binary = {}; // number is storred right to left, e.g. most significant bits are at the end of an array
                                                          // can't use bitset here, because not constexp (since C++23)
        size_t words_count = 0;

    private:
        template<size_t s>
        friend class BigInt;

        template<IntIter It>
        static constexpr size_t RangeWordsCount(It begin, It end) noexcept; 

        template<IntIter It>
        constexpr void UnsignedResetBinary(It begin, It end) noexcept;

        template<IntIter It>
        constexpr void UnsignedAddRange(It begin, It end) noexcept;

        template<IntIter It>
        constexpr void UnsignedSubSmallerRange(It begin, It end) noexcept;

        template<IntIter It>
        constexpr void UnsignedShortMultiplyByRange(It begin, It end) noexcept; // either this or rhs is lesser than 32 bits

        template<size_t subint_words_capacity, IntIter It>
        constexpr void KaratsubaMultiplyByRange(It begin, It end) noexcept;

        template<size_t small_words_capacity>
        constexpr void KaratsubaMultiply(const BigInt&) noexcept;

        template<IntIter It>
        constexpr void UnsignedMultiplyByRange(It begin, It end) noexcept;

        constexpr BigInt ShortUnsignedDivideBy(uint32_t) noexcept; // returns remainder

        template<IntIter It>
        constexpr BigInt UnsignedDivideByRange(It begin, It end) noexcept; // returns remainder

        constexpr BigInt UnsignedDivideBy(const BigInt&) noexcept; // returns remainder

        constexpr void PowerInner(BigInt& res, BigInt&& exp) const noexcept;
        constexpr void PowerInner(BigInt& res, BigInt&& exp, const BigInt& mod) const noexcept;
    };

    // impl
    template<size_t words_capacity>
    constexpr BigInt<words_capacity>::BigInt(int64_t from) noexcept {
        if (from < 0) {
            is_positive = false;
            from = -from;
        }

        if ((binary[0] = from & ((1ull << 32) - 1))) {
            words_count = 1;
        }

        if (words_capacity > 1) {
            if ((binary[1] = from >> 32)) {
                words_count = 2;
            }
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>::BigInt(std::string_view str) noexcept {
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
                if (str[i] != '0' && str[i] != '1' && str[i] != '\'') {
                    std::cerr << "Invalid binary format" << std::endl;
                    std::terminate();
                }

                size_t idx = str.size() - 1 - i;
                if (str[idx] == '1') {
                    set_bit(count);
                }

                if (str[idx] != '\'') {
                    count += 1;
                    if (count >= words_capacity * 32) {
                        std::cerr << "String is too long" << std::endl;
                        std::terminate();
                    }
                }
            }
        } else if (str.size() > 0) {
            const uint32_t ten[] = {10};
            uint32_t buffer[] = {0};

            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '\'') {
                    continue;
                }

                if (str[i] >= '0' && str[i] <= '9') {
                    buffer[0] = str[i] - '0';
                    UnsignedShortMultiplyByRange(ten, ten + 1);
                    UnsignedAddRange(buffer, buffer + 1);
                } else {
                    std::cerr << "Invalid decimal format" << std::endl;
                    std::terminate();
                }
            }
        }
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr BigInt<words_capacity>::BigInt(It begin, It end, bool is_positive) noexcept
        : is_positive{is_positive}
    {
        size_t counter = 1;
        auto set = binary.begin();
        for (It it = begin; it != end; ++it, ++set, ++counter) {
            if (counter > words_capacity) {
                std::cerr << "Range is too big" << std::endl;
                std::terminate();
            }

            if ((*set = *it)) {
                words_count = counter;
            }
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator<<=(size_t shift) noexcept {
        if (shift >= words_capacity * 32) {
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
        for (size_t i = 0; i < words_capacity; ++i) {
            size_t idx = words_capacity - 1 - i;
            if ((binary[idx] = get_shifted_word(idx)) && words_count == 0) {
                words_count = idx + 1;
            }
        }
        return *this;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator>>=(size_t shift) noexcept {
        if (shift >= words_capacity * 32) {
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
            if (size_t shifted_idx = idx + word_offset; shifted_idx < words_capacity) {
                ret |= binary[shifted_idx] >> bit_offset;
                if (bit_offset != 0 && shifted_idx + 1 < words_capacity) [[likely]] {
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

    template<size_t words_capacity>
    constexpr BigInt<words_capacity> BigInt<words_capacity>::operator<<(size_t shift_int) const noexcept {
        return BigInt{*this} <<= shift_int;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity> BigInt<words_capacity>::operator>>(size_t shift_int) const noexcept {
        return BigInt{*this} >>= shift_int;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity> BigInt<words_capacity>::operator-() const noexcept {
        BigInt copy = *this;
        copy.is_positive ^= true;
        return copy;
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr void BigInt<words_capacity>::UnsignedResetBinary(It begin, It end) noexcept {
        words_count = 0;
        size_t counter = 0;
        while (begin != end) {
            if ((binary[counter] = *begin)) {
                words_count = counter + 1;
            }
            ++counter;
            ++begin;
        }
        while (counter < words_capacity) {
            binary[counter] = 0;
            ++counter;
        }
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr void BigInt<words_capacity>::UnsignedAddRange(It begin, It end) noexcept {
        constexpr uint32_t first_bit_mask = 1u << 31;
        uint32_t carry = 0;

        size_t old_words_count = words_count;
        size_t i = 0;
        while (begin != end) {
            if (i >= words_capacity) {
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

        while (carry != 0 && i < words_capacity) {
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
            std::cerr << "Addition overflow" << std::endl;
            std::terminate();
        }

        words_count = std::max(old_words_count, words_count);
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator+=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            return *this -= -rhs;
        }
        UnsignedAddRange(rhs.binary.begin(), rhs.binary.begin() + rhs.words_count);
        return *this;
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr void BigInt<words_capacity>::UnsignedSubSmallerRange(It begin, It end) noexcept {
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
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator-=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            UnsignedAddRange(rhs.cbegin(), rhs.cend());
            return *this;
        }

        if (auto cmp = *this <=> rhs; cmp == 0) {
            return *this = {};
        } else if ((cmp > 0 && !is_positive) || (cmp < 0 && is_positive)) {
            return *this = -(rhs - *this);
        }

        UnsignedSubSmallerRange(rhs.binary.begin(), rhs.binary.begin() + rhs.words_count);
        return *this;
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr void BigInt<words_capacity>::UnsignedShortMultiplyByRange(It begin, It end) noexcept {
        constexpr uint32_t first_bit_mask = 1u << 31;

        // It is possible that &rhs == this, so values are storred in buffer
        uint32_t lhs_word_buf = binary[0];
        uint32_t rhs_word_buf = *begin;

        const size_t old_words_count = words_count;
        const size_t iterations = std::max(words_count, RangeWordsCount(begin, end));
        words_count = 0;
        binary[0] = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if (begin != end) {
                ++begin; // value already stored in buffer
            }
            uint64_t prod = lhs_word_buf;
            prod *= rhs_word_buf;
            if (i + 1 < words_capacity) {
                if (old_words_count == 1 && begin != end) {
                    rhs_word_buf = *begin;
                } else {
                    lhs_word_buf = binary[i + 1];
                }
            }
            uint32_t prod_l = static_cast<uint32_t>(prod & kMaxWord);
            uint32_t prod_h = static_cast<uint32_t>(prod >> 32);

            if (i + 1 < words_capacity) {
                binary[i + 1] = prod_h;
            } else if (prod_h) {
                std::cerr << "Multiplication Overflow" << std::endl;
                std::terminate();
            }

            if (kMaxWord - prod_l < binary[i]) {
                if (prod_l >= first_bit_mask) {
                    prod_l ^= first_bit_mask;
                } else {
                    binary[i] ^= first_bit_mask;
                }
                binary[i] += prod_l;
                binary[i] ^= first_bit_mask;

                if (i + 1 < words_capacity) {
                    binary[i + 1] += 1;
                } else {
                    std::cerr << "Multiplication overflow" << std::endl;
                    std::terminate();
                }
            } else {
                if ((binary[i] += prod_l)) {
                    words_count = i + 1;
                }
            }
        }

        if (iterations < words_capacity && binary[iterations]) {
            words_count = iterations + 1;
        }
    }

    template<size_t _>
    template<size_t subint_words_capacity, IntIter It>
    constexpr void BigInt<_>::KaratsubaMultiplyByRange(It begin, It end) noexcept {
        using SmallInt = BigInt<subint_words_capacity>;

        size_t range_words_count = RangeWordsCount(begin, end);
        size_t end_thr = std::max(words_count, range_words_count);
        size_t mid_thr = (end_thr + 1) / 2;
        SmallInt this_l {cbegin(), cbegin() + mid_thr};
        SmallInt this_h {cbegin() + mid_thr, cbegin() + end_thr};

        SmallInt rhs_l;
        SmallInt rhs_h;

        if (mid_thr >= range_words_count) {
            rhs_l.UnsignedResetBinary(begin, begin + range_words_count);
        } else {
            rhs_l.UnsignedResetBinary(begin, begin + mid_thr);
            rhs_h.UnsignedResetBinary(begin + mid_thr, begin + range_words_count);
        }

        SmallInt mix = (this_l + this_h) * (rhs_l + rhs_h);
        this_h *= rhs_h;
        this_l *= rhs_l;
        size_t shift = mid_thr * 32;
        UnsignedResetBinary(this_h.cbegin(), this_h.cend());
        *this <<= shift;
        UnsignedAddRange(mix.cbegin(), mix.cend());
        UnsignedSubSmallerRange(this_h.cbegin(), this_h.cend());
        UnsignedSubSmallerRange(this_l.cbegin(), this_l.cend());
        *this <<= shift;
        UnsignedAddRange(this_l.cbegin(), this_l.cend());
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr void BigInt<words_capacity>::UnsignedMultiplyByRange(It begin, It end) noexcept {
        size_t range_words_count = RangeWordsCount(begin, end);

        if (words_count == 1 || range_words_count == 1) {
            // Case when multiplication can be done linearly, without extra stack allocation for temporary result
            UnsignedShortMultiplyByRange(begin, begin + range_words_count);
        } else if constexpr (words_capacity <= 40) {
            // for small numbers do quadratic multiplication
            size_t old_words_count = words_count;
            std::array<uint32_t, words_capacity> binary_copy = binary;
            binary = {};
            for (size_t i = 0; i < old_words_count; ++i) {
                BigInt tmp {begin, begin + range_words_count};
                tmp.UnsignedShortMultiplyByRange(binary_copy.begin() + i, binary_copy.begin() + i + 1);
                tmp <<= i * 32;
                UnsignedAddRange(tmp.cbegin(), tmp.cend());
            }
        } else {
            // https://www.geeksforgeeks.org/karatsuba-algorithm-for-fast-multiplication-using-divide-and-conquer-algorithm/
            if (size_t max_size = words_count + range_words_count; max_size <= words_capacity / 16) {
                KaratsubaMultiplyByRange<words_capacity / 16 + 1>(begin, begin + range_words_count);
            } else if (max_size <= words_capacity / 8) {
                KaratsubaMultiplyByRange<words_capacity / 8 + 1>(begin, begin + range_words_count);
            } else if (max_size <= words_capacity / 4) {
                KaratsubaMultiplyByRange<words_capacity / 4 + 1>(begin, begin + range_words_count);
            } else if (max_size <= words_capacity / 2) {
                KaratsubaMultiplyByRange<words_capacity / 2 + 1>(begin, begin + range_words_count);
            } else {
                KaratsubaMultiplyByRange<words_capacity>(begin, begin + range_words_count);
            }
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator*=(const BigInt& rhs) noexcept {
        if (IsZero() || rhs.IsZero()) {
            is_positive = true;
            binary = {};
            words_count = 0;
            return *this;
        } else if (rhs.IsOne()) {
            return *this;
        } else if (IsOne()) {
            if (this != &rhs) {
                is_positive = rhs.is_positive;
                std::copy(rhs.binary.begin(), rhs.binary.begin() + rhs.words_count, binary.begin());
                words_count = rhs.words_count;
            }
            return *this;
        } else if (rhs.IsPowerOf2()) {
            return *this <<= rhs.BitWidth() - 1;
        }

        is_positive ^= !rhs.is_positive;
        UnsignedMultiplyByRange(rhs.cbegin(), rhs.cend());
        return *this;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity> BigInt<words_capacity>::ShortUnsignedDivideBy(uint32_t rhs) noexcept {
        BigInt remainder;
        bool set = false;
        const size_t iterations = words_count;
        words_count = 0;
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

        if ((remainder.binary[0] = static_cast<uint32_t>(window))) {
            remainder.words_count = 1;
        }
        return remainder;
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr BigInt<words_capacity> BigInt<words_capacity>::UnsignedDivideByRange(It begin, It end) noexcept {
        size_t range_words_count = RangeWordsCount(begin, end);
        if (range_words_count > words_count) {
            std::cerr << "Dividing by larger number" << std::endl;
            std::terminate();
        }

        if (range_words_count == 1) {
            return ShortUnsignedDivideBy(*begin);
        }

        auto unsigned_cmp = [](It lhs_begin, It lhs_end, It rhs_begin, It rhs_end) {
            if (auto cmp = lhs_end - lhs_begin <=> rhs_end - rhs_begin; cmp != 0) {
                return cmp;
            }

            It lhs_rend = lhs_end - 1;
            It rhs_rend = rhs_end - 1;
            while (lhs_rend != lhs_begin) {
                if (auto cmp = *lhs_rend <=> *rhs_rend; cmp != 0) {
                    return cmp;
                }
            }

            return *lhs_begin <=> *rhs_begin;
        };

        if (words_count - range_words_count <= 1) {
            // binary search
            uint32_t l = 0;
            uint32_t r = kMaxWord;
            uint32_t m[2];
            while (l != r) {
                m[0] = (l / 2) + (r / 2) + (l & r & 1);
                BigInt<words_capacity + 1> tmp {m[0]};
                tmp.UnsignedMultiplyByRange(begin, begin + range_words_count);
                if (auto cmp = unsigned_cmp(tmp.cbegin(), tmp.cend(), cbegin(), cend()); cmp == 0) {
                    UnsignedResetBinary(m, m + 1);
                    return 0;
                } else if (cmp > 0) {
                    r = m[0] - 1;
                } else {
                    BigInt remainder = *this;
                    remainder.UnsignedSubSmallerRange(tmp.cbegin(), tmp.cend());
                    if (unsigned_cmp(remainder.cbegin(), remainder.cend(), begin, begin + range_words_count) < 0) {
                        UnsignedResetBinary(m, m + 1);
                        return remainder;
                    } else {
                        l = m[0] + 1;
                    }
                }
            }
            m[0] = l;
            BigInt tmp {m[0]};
            tmp.UnsignedMultiplyByRange(begin, begin + range_words_count);
            BigInt remainder = *this;
            remainder.UnsignedSubSmallerRange(tmp.cbegin(), tmp.cend());
            UnsignedResetBinary(m, m + 1);
            return remainder;
        } else {
            BigInt{begin, end}.PrintWords();
            BigInt remainder = *this;
            const size_t old_words_count = words_count;
            UnsignedResetBinary(end, end);
            BigInt tmp;
            for (size_t i = 0; i < old_words_count; ++i) {
                size_t idx = old_words_count - 1 - i;
                tmp <<= 32;
                tmp.UnsignedAddRange(remainder.cbegin() + idx, remainder.cbegin() + idx + 1);
                if (unsigned_cmp(tmp.cbegin(), tmp.cend(), begin, begin + range_words_count) >= 0) {
                    if (tmp.words_count - range_words_count > 1) {
                        std::cerr << "Unreachable" << std::endl;
                        std::terminate();
                    }
                    BigInt small_remainder = tmp.UnsignedDivideByRange(begin, begin + range_words_count);
                    if (tmp.words_count > 1) {
                        std::cerr << "Unreachable" << std::endl;
                        std::terminate();
                    }
                    *this <<= 32;
                    UnsignedAddRange(tmp.cbegin(), tmp.cend());
                    small_remainder <<= idx * 32;
                    remainder.UnsignedSubSmallerRange(small_remainder.cbegin(), small_remainder.cend());
                    tmp.UnsignedResetBinary(small_remainder.cbegin(), small_remainder.cend());
                } else {
                    *this <<= 32;
                }
            }
            return remainder;
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator/=(const BigInt& rhs) noexcept {
        if (rhs.IsZero()) {
            std::cerr << "Dividing by 0" << std::endl;
            std::terminate();
        } else if (IsZero() || rhs.IsOne()) {
            return *this;
        } else if (words_count < rhs.words_count) {
            return *this = 0;
        }

        is_positive ^= !rhs.is_positive;
        if (rhs.IsPowerOf2()) {
            *this >>= rhs.BitWidth() - 1;
        } else {
            UnsignedDivideByRange(rhs.cbegin(), rhs.cend());
        }
        return *this;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator%=(const BigInt& rhs) noexcept {
        if (rhs.IsZero()) {
            std::cerr << "Dividing by 0" << std::endl;
            std::terminate();
        } else if (rhs.IsOne() || !rhs.is_positive) {
            std::cerr << "Remainder of dividing by 1 or by negative number" << std::endl;
            std::terminate();
        } else if (IsZero() || words_count < rhs.words_count) {
            return *this;
        } else if (rhs.IsPowerOf2()) {
            return *this &= rhs - 1;
        }

        BigInt remainder = UnsignedDivideByRange(rhs.cbegin(), rhs.cend());
        UnsignedResetBinary(remainder.cbegin(), remainder.cend());
        return *this;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity> BigInt<words_capacity>::operator~() const noexcept {
        BigInt ret;
        ret.is_positive = is_positive;
        for (size_t i = 0; i < words_capacity; ++i) {
            if ((ret.binary[i] = ~binary[i])) {
                ret.words_count = i + 1;
            }
        }

        return ret;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator^=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if ((binary[i] ^= other.binary[i])) {
                words_count = i + 1;
            }
        }
        return *this;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator|=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = iterations;
        for (size_t i = 0; i < iterations; ++i) {
            binary[i] |= other.binary[i];
        }
        return *this;
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator&=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if ((binary[i] &= other.binary[i])) {
                words_count = i + 1;
            }
        }
        return *this;
    }

    template<size_t _>
    constexpr BigInt<_> BigInt<_>::Power(BigInt exp) const noexcept {
        BigInt ret;
        PowerInner(ret, std::move(exp));
        return ret;
    }

    template<size_t _>
    constexpr BigInt<_> BigInt<_>::Power(BigInt exp, const BigInt& mod) const noexcept {
        BigInt ret;
        PowerInner(ret, std::move(exp), mod);
        if (ret >= mod) {
            ret %= mod;
        }
        return ret;
    }

    template<size_t _>
    constexpr void BigInt<_>::PowerInner(BigInt& res, BigInt&& exp) const noexcept {
        if (exp.IsZero()) {
            res = 1;
        } else if (exp.IsOne()) {
            res = *this;
        } else {
            bool isOdd = exp.binary[0] & 1;
            PowerInner(res, std::move(exp >>= 1));
            res *= res;
            if (isOdd) {
                res *= *this;
            }
        }
    }

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::PowerInner(BigInt& res, BigInt&& exp, const BigInt& mod) const noexcept {
        // Might return value bigger than mod
        if (exp.IsZero()) {
            res = 1;
        } else if (exp.IsOne()) {
            res = *this;
        } else {
            bool isOdd = exp.binary[0] & 1;
            PowerInner(res, std::move(exp >>= 1), mod);
            if (res.words_count * 2 > words_capacity) {
                BigInt<2 * words_capacity> tmp {res.cbegin(), res.cend()};
                tmp *= tmp;
                tmp %= {mod.cbegin(), mod.cend()};
                res.UnsignedResetBinary(tmp.cbegin(), tmp.cend());
            } else {
                res *= res;
            }
            if (isOdd) {
                if (res.words_count + words_count > words_capacity) {
                    BigInt<2 * words_capacity> tmp {res.cbegin(), res.cend()};
                    tmp.UnsignedMultiplyByRange(cbegin(), cend());
                    tmp %= {mod.cbegin(), mod.cend()};
                    res.UnsignedResetBinary(tmp.cbegin(), tmp.cend());
                } else {
                    res *= *this;
                }
            }
        }
    }

    template<size_t _>
    constexpr bool BigInt<_>::IsZero() const noexcept {
        return words_count == 0;
    }

    template<size_t _>
    constexpr bool BigInt<_>::IsOne() const noexcept {
        return is_positive && words_count == 1 && binary[0] == 1;
    }

    template<size_t _>
    constexpr bool BigInt<_>::IsPowerOf2() const noexcept {
        if (IsZero()) {
            return false;
        }

        for (size_t i = 0; i < words_count - 1; ++i) {
            if (binary[i]) {
                return false;
            }
        }

        return (binary[words_count - 1] & (binary[words_count - 1] - 1)) == 0;
    }

    template<size_t words_capacity>
    constexpr size_t BigInt<words_capacity>::BitWidth() const noexcept {
        if (IsZero()) {
            return 0;
        }

        return (words_count - 1) * 32 + std::bit_width(binary[words_count - 1]);
    }

    template<size_t words_capacity>
    constexpr const uint32_t* BigInt<words_capacity>::cbegin() const noexcept {
        return binary.cbegin();
    }

    template<size_t words_capacity>
    constexpr const uint32_t* BigInt<words_capacity>::cend() const noexcept {
        return binary.cbegin() + words_count;
    }

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::PrintWords() const noexcept {
        std::cerr << words_count << " ";
        if (is_positive) {
            std::cerr << "+ ";
        } else {
            std::cerr << "- ";
        }
        for (size_t i = 0; i < words_capacity; ++i) {
            if (i != 0) {
                std::cerr << "'";
            }
            std::cerr << binary[words_capacity - 1 - i];
        }
        std::cerr << std::endl;
    }

    template<size_t words_capacity>
    template<IntIter It>
    constexpr size_t BigInt<words_capacity>::RangeWordsCount(It begin, It end) noexcept {
        if constexpr (std::random_access_iterator<It>) {
            It it = end - 1;
            size_t counter = end - begin;
            if (counter > words_capacity) {
                std::cerr << "Range is bigger than capacity" << std::endl;
                std::terminate();
            }

            while (it != begin) {
                if (*it != 0) {
                    return counter;
                }
                counter -= 1;
            }
            if (*begin) {
                return 1;
            } else {
                return 0;
            }
        } else {
            size_t counter = 1;
            size_t ret = 0;
            for (It it = begin; it != end; ++it) {
                if (counter >= words_capacity) {
                    std::cerr << "Range is bigger than capacity" << std::endl;
                    std::terminate();
                }

                if (*it != 0) {
                    ret = counter;
                }
                counter += 1;
            }
            return ret;
        }
    }

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::PrintDecimal() const noexcept {
        char str[words_capacity * 32];
        size_t counter = 0;
        BigInt copy = *this;
        while (!copy.IsZero()) {
            str[counter] = '0' + static_cast<char>((copy % 10).binary[0]);
            copy /= 10;
            ++counter;
        }
        char reversed[words_capacity * 32];
        for (size_t i = 0; i < counter; ++i) {
            reversed[i] = str[counter - 1 - i];
        }
        reversed[counter] = '\0';
        std::cerr << reversed << std::endl;
    }

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::PrintBinary() const noexcept {
        std::cerr << words_count << " ";
        if (is_positive) {
            std::cerr << "+ ";
        } else {
            std::cerr << "- ";
        }
        for (size_t i = 0; i < words_capacity; ++i) {
            uint32_t n = 1u << 31;
            while (n != 0) {
                std::cerr << (binary[words_capacity - 1 - i] & n ? 1 : 0);
                n >>= 1;
            }
        }
        std::cerr << std::endl;
    }

    template<size_t words_capacity>
    constexpr bool BigInt<words_capacity>::operator==(const BigInt& rhs) const noexcept {
        PrintWords();
        rhs.PrintWords();
        return (*this <=> rhs) == 0;
    }

    template<size_t words_capacity>
    constexpr auto BigInt<words_capacity>::operator<=>(const BigInt& rhs) const noexcept {
        if (IsZero() && rhs.IsZero()) {
            return std::strong_ordering::equal;
        } else if (IsZero()) {
            return rhs.is_positive ? std::strong_ordering::less : std::strong_ordering::greater;
        } else if (IsZero()) {
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

        for (size_t i = 0; i < words_count; ++i) {
            size_t idx = words_count - 1 - i;
            if (binary[idx] != rhs.binary[idx]) {
                if (is_positive) {
                    return binary[idx] <=> rhs.binary[idx];
                } else {
                    return rhs.binary[idx] <=> binary[idx];
                }
            }
        }

        return std::strong_ordering::equal;
    }

    // Arithmetic opeartors
    template<size_t c>
    constexpr BigInt<c> operator+(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs += rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator+(const BigInt<c>& lhs, T&& rhs) noexcept {
        return BigInt<c>{std::forward<T>(rhs)} + lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator+(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} + rhs;
    }

    template<size_t c>
    constexpr BigInt<c> operator-(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs -= rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator-(const BigInt<c>& lhs, T&& rhs) noexcept {
        return -(BigInt<c>{std::forward<T>(rhs)} - lhs);
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator-(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} - rhs;
    }

    template<size_t c>
    constexpr BigInt<c> operator*(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator*(const BigInt<c>& lhs, T&& rhs) noexcept {
        return BigInt<c>{std::forward<T>(rhs)} * lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator*(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} * rhs;
    }

    template<size_t c>
    constexpr BigInt<c> operator/(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs /= rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator/(BigInt<c> lhs, T&& rhs) noexcept {
        lhs /= BigInt<c>{std::forward<T>(rhs)};
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator/(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} / rhs;
    }

    template<size_t c>
    constexpr BigInt<c> operator%(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs %= rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator%(BigInt<c> lhs, T&& rhs) noexcept {
        lhs %= BigInt<c>{std::forward<T>(rhs)};
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator%(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} % rhs;
    }

    template<size_t c>
    constexpr BigInt<c> operator&(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs &= rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator&(const BigInt<c>& lhs, T&& rhs) noexcept {
        return BigInt<c>{std::forward<T>(rhs)} & lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator&(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} & rhs;
    }

    template<size_t c>
    constexpr BigInt<c> operator|(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs |= rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator|(const BigInt<c>& lhs, T&& rhs) noexcept {
        return BigInt<c>{std::forward<T>(rhs)} | lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator|(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} | rhs;
    }

    template<size_t c>
    constexpr BigInt<c> operator^(BigInt<c> lhs, const BigInt<c>& rhs) noexcept {
        lhs ^= rhs;
        return lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator^(const BigInt<c>& lhs, T&& rhs) noexcept {
        return BigInt<c>{std::forward<T>(rhs)} ^ lhs;
    }

    template<size_t c, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c> operator^(T&& lhs, const BigInt<c>& rhs) noexcept {
        return BigInt<c>{std::forward<T>(lhs)} ^ rhs;
    }

} // namespace algo
