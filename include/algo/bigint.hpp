#pragma once

#include <helpers/assert.hpp>
#include <helpers/concepts.hpp>

#include <compare>
#include <algorithm>
#include <iterator>
#include <limits>
#include <type_traits>
#include <array>
#include <bit>

namespace algo {

    template<size_t words_capacity>
    class BigInt {
    public:
        using Word = uint32_t;
        static constexpr size_t kWordBSize = sizeof(Word) * std::numeric_limits<char8_t>::digits;
        static_assert(words_capacity > 0 && !std::numeric_limits<Word>::is_signed && std::numeric_limits<Word>::is_modulo);

        constexpr BigInt() noexcept = default;
        constexpr BigInt(int64_t integer) noexcept;
        constexpr BigInt(std::string_view str) noexcept;

        constexpr BigInt(const Range<Word> auto& range, bool is_positive = true) noexcept;

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

        constexpr bool IsZero() const noexcept;
        constexpr bool IsOne() const noexcept;
        constexpr bool IsPowerOf2() const noexcept;
        constexpr size_t BitWidth() const noexcept;

        constexpr const Word* cbegin() const noexcept;
        constexpr const Word* cend() const noexcept;

        std::array<Word, words_capacity> binary = {}; // number is storred right to left, e.g. most significant bits are at the end of an array
                                                      // can't use bitset here, because not constexpr (since C++23)
        size_t words_count = 0;
        bool is_positive = true;

    private:
        template<size_t s>
        friend class BigInt;

        static constexpr size_t RangeWordsCount(const Range<Word> auto& range) noexcept; 

        // All operations below can work properly if range points to subrange of this->binary
        constexpr void UnsignedResetBinary(const Range<Word> auto& range) noexcept;
        constexpr void UnsignedAddRange(const Range<Word> auto& range) noexcept;
        constexpr void UnsignedSubSmallerRange(const Range<Word> auto& range) noexcept;
        constexpr void UnsignedShortMultiplyByRange(const Range<Word> auto& range) noexcept; // either this or rhs is lesser than 32 bits
        template<size_t subint_words_capacity>
        constexpr void KaratsubaMultiplyByRange(const Range<Word> auto& range) noexcept;
        constexpr void UnsignedMultiplyByRange(const Range<Word> auto& range) noexcept;
        constexpr BigInt ShortUnsignedDivideBy(uint32_t) noexcept; // returns remainder
        constexpr BigInt UnsignedDivideByRange(const Range<Word> auto& range) noexcept; // returns remainder

        constexpr void PowerInner(BigInt& res, BigInt&& exp) const noexcept;

        static constexpr Word kMaxWord = std::numeric_limits<Word>::max();
    };

    // Traits
    template<typename T>
    struct IsBigInt : std::false_type {};

    template<size_t S>
    struct IsBigInt<BigInt<S>> : std::true_type {};

    // Implementation
    template<size_t words_capacity>
    constexpr BigInt<words_capacity>::BigInt(int64_t sfrom) noexcept {
        ASSERT(words_capacity > 1 || sfrom <= kMaxWord, "Given integer won't fit in provided type");

        if (sfrom == 0) {
            return;
        }

        uint64_t from;
        if (sfrom < 0) {
            is_positive = false;
            if (sfrom == std::numeric_limits<int64_t>::min()) {
                from = 1ull << 63;
            } else {
                from = static_cast<uint64_t>(-sfrom);
            }
        } else {
            from = sfrom;
        }

        binary[0] = from & kMaxWord;
        if (from > kMaxWord) {
            binary[1] = from >> kWordBSize;
            words_count = 2;
        } else {
            words_count = 1;
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>::BigInt(std::string_view str) noexcept {
        ASSERT(str.size() != 0);

        if (str[0] == '-') {
            is_positive = false;
            str.remove_prefix(1);
        }

        if (str.size() >= 2 && str[0] == '0' && str[1] == 'b') {
            str.remove_prefix(2);
            auto set_bit = [this](size_t bit_idx) {
                binary[bit_idx / kWordBSize] |= 1u << (bit_idx % kWordBSize);
                words_count = bit_idx / kWordBSize + 1;
            };
            size_t count = 0;
            for (size_t i = 0; i < str.size(); ++i) {
                ASSERT(str[i] == '0' || str[i] == '1' || str[i] == '\'');

                size_t idx = str.size() - 1 - i;
                if (str[idx] == '1') {
                    set_bit(count);
                }

                if (str[idx] != '\'') {
                    count += 1;
                    ASSERT(count <= words_capacity * kWordBSize, "Type is too small to fit provided string");
                }
            }
        } else if (str.size() > 0) {
            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '\'') {
                    continue;
                }

                ASSERT(str[i] >= '0' && str[i] <= '9');
                UnsignedShortMultiplyByRange(std::ranges::single_view(static_cast<Word>(10)));
                UnsignedAddRange(std::ranges::single_view(static_cast<Word>(str[i] - '0')));
            }
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>::BigInt(const Range<Word> auto& range, bool is_positive) noexcept
        : is_positive{is_positive}
    {
        size_t counter = 1;
        for (auto it = std::ranges::begin(range); it != std::ranges::end(range); ++it) {
            ASSERT(counter <= words_capacity, "Type is too small for provided range");
            if (*it > 0) {
                words_count = counter;
                binary[counter - 1] = *it;
            }

            ++counter;
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator<<=(size_t shift) noexcept {
        ASSERT(shift < words_capacity * kWordBSize, "Shift is bigger than bit size");

        if (shift == 0) {
            return *this;
        }

        size_t word_offset = shift / kWordBSize;
        size_t bit_offset = shift % kWordBSize;
        auto get_shifted_word = [&](size_t idx) -> uint32_t {
            uint32_t ret = 0;
            if (idx >= word_offset) {
                ret |= binary[idx - word_offset] << bit_offset;
                if (bit_offset != 0 && idx > word_offset) [[likely]] {
                    ret |= binary[idx - word_offset - 1] >> (kWordBSize - bit_offset);
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
        ASSERT(shift < words_capacity * kWordBSize, "Shift is bigger than bit size");
        if (shift == 0) {
            return *this;
        }
        size_t word_offset = shift / kWordBSize;
        size_t bit_offset = shift % kWordBSize;
        auto get_shifted_word = [&](size_t idx) -> uint32_t {
            uint32_t ret = 0;
            if (size_t shifted_idx = idx + word_offset; shifted_idx < words_capacity) {
                ret |= binary[shifted_idx] >> bit_offset;
                if (bit_offset != 0 && shifted_idx + 1 < words_capacity) [[likely]] {
                    ret |= binary[shifted_idx + 1] << (kWordBSize - bit_offset);
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
    constexpr void BigInt<words_capacity>::UnsignedResetBinary(const Range<Word> auto& range) noexcept {
        words_count = 0;
        size_t counter = 0;
        binary = {};
        for (auto it = std::ranges::begin(range); it != std::ranges::end(range); ++it) {
            if (*it > 0) {
                binary[counter] = *it;
                words_count = counter + 1;
            }

            ++counter;
        }
    }

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::UnsignedAddRange(const Range<Word> auto& range) noexcept {
        Word carry = 0;

        size_t range_words_count = RangeWordsCount(range);
        size_t i = 0;
        auto it = std::ranges::begin(range);
        for (; i < range_words_count; ++i, ++it) {
            if (*it != kMaxWord || carry == 0) {
                Word tmp = *it + carry;
                if (kMaxWord - binary[i] < tmp) {
                    carry = 1;
                } else {
                    carry = 0;
                }
                binary[i] += tmp; // safe to do so because Word is is_modulo type
            }
        }

        while (carry != 0 && i < words_capacity) {
            if (kMaxWord == binary[i]) {
                binary[i] = 0;
            } else {
                binary[i] += carry;
                carry = 0;
            }
            ++i;
        }

        ASSERT(carry == 0, "Addition overflow");
        if (i > words_count) {
            words_count = i;
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator+=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            is_positive ^= true;
            *this -= rhs;
            is_positive ^= true;
            return *this;
        } else {
            UnsignedAddRange(std::ranges::subrange(rhs.cbegin(), rhs.cend()));
            return *this;
        }
    }

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::UnsignedSubSmallerRange(const Range<Word> auto& range) noexcept {
        Word borrow = 0;

        size_t mb_words_count = 0;
        size_t range_words_count = RangeWordsCount(range);
        ASSERT(range_words_count <= words_count, "Subtracting greater value");

        size_t i = 0;
        auto it = std::ranges::begin(range);
        for (; i < range_words_count; ++i, ++it) {
            if (*it != kMaxWord || borrow == 0) {
                Word tmp = *it + borrow;
                if (binary[i] < tmp) {
                    borrow = 1;
                } else {
                    borrow = 0;
                }
                binary[i] -= tmp;
                if (binary[i]) {
                    mb_words_count = i + 1;
                }
            }
        }

        while (borrow != 0 && i < words_count) {
            if (binary[i] == 0) {
                binary[i] = kMaxWord;
                mb_words_count = i + 1;
            } else {
                binary[i] -= borrow;
                borrow = 0;
                if (binary[i]) {
                    mb_words_count = i + 1;
                }
            }
            ++i;
        }

        ASSERT(borrow == 0, "Subtracting greater value");

        if (words_count > 0 && binary[words_count - 1] == 0) {
            words_count = mb_words_count;
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator-=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            UnsignedAddRange(std::ranges::subrange(rhs.cbegin(), rhs.cend()));
            return *this;
        }

        if (auto cmp = *this <=> rhs; cmp == 0) {
            return *this = {};
        } else if ((cmp > 0 && !is_positive) || (cmp < 0 && is_positive)) { // TODO Make subtraction for any range
            return *this = -(rhs - *this);
        }

        UnsignedSubSmallerRange(std::ranges::subrange(rhs.cbegin(), rhs.cend()));
        return *this;
    }

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::UnsignedShortMultiplyByRange(const Range<Word> auto& range) noexcept {
        using DoubleWord = uint64_t;
        static_assert(sizeof(DoubleWord) >= 2 * sizeof(Word), "Double word should be at least two times bigger than word");

        const size_t old_words_count = words_count;
        const size_t range_words_count = RangeWordsCount(range);
        ASSERT(old_words_count <= 1 || range_words_count <= 1, "Short multiplication not applicable");

        const size_t iterations = std::max(words_count, range_words_count);

        auto begin = std::ranges::begin(range);
        auto end = std::ranges::end(range);

        // It is possible that &rhs == this, so values are storred in buffer
        Word lhs_word_buf = binary[0];
        Word rhs_word_buf = *begin;

        binary[0] = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if (begin != end) {
                ++begin; // value already stored in buffer
            }
            DoubleWord prod = lhs_word_buf;
            prod *= rhs_word_buf;
            if (i + 1 < words_capacity) {
                if (begin != end) {
                    rhs_word_buf = *begin;
                } else {
                    lhs_word_buf = binary[i + 1];
                }
            }
            Word prod_l = static_cast<Word>(prod & kMaxWord);
            Word prod_h = static_cast<Word>(prod >> kWordBSize);

            if (i + 1 < words_capacity) {
                binary[i + 1] = prod_h;
            } else {
                ASSERT(prod_h == 0, "Multiplication overflow");
            }

            if (kMaxWord - prod_l < binary[i]) {
                ASSERT(i + 1 < words_capacity, "Multiplication overflow");
                binary[i + 1] += 1;
            }
            binary[i] += prod_l;
        }

        if (iterations < words_capacity && binary[iterations]) {
            words_count = iterations + 1;
        } else {
            words_count = iterations;
        }
    }

    //template<size_t _>
    //template<size_t subint_words_capacity, IntIter It>
    //constexpr void BigInt<_>::KaratsubaMultiplyByRange(It begin, It end) noexcept {
        //using SmallInt = BigInt<subint_words_capacity>;

        //size_t range_words_count = RangeWordsCount(begin, end);
        //size_t end_thr = std::max(words_count, range_words_count);
        //size_t mid_thr = (end_thr + 1) / 2;
        //SmallInt this_l {cbegin(), cbegin() + mid_thr};
        //SmallInt this_h {cbegin() + mid_thr, cbegin() + end_thr};

        //SmallInt rhs_l;
        //SmallInt rhs_h;

        //if (mid_thr >= range_words_count) {
            //rhs_l.UnsignedResetBinary(begin, begin + range_words_count);
        //} else {
            //rhs_l.UnsignedResetBinary(begin, begin + mid_thr);
            //rhs_h.UnsignedResetBinary(begin + mid_thr, begin + range_words_count);
        //}

        //SmallInt mix = (this_l + this_h) * (rhs_l + rhs_h);
        //this_h *= rhs_h;
        //this_l *= rhs_l;
        //size_t shift = mid_thr * 32;
        //UnsignedResetBinary(this_h.cbegin(), this_h.cend());
        //*this <<= shift;
        //UnsignedAddRange(mix.cbegin(), mix.cend());
        //UnsignedSubSmallerRange(this_h.cbegin(), this_h.cend());
        //UnsignedSubSmallerRange(this_l.cbegin(), this_l.cend());
        //*this <<= shift;
        //UnsignedAddRange(this_l.cbegin(), this_l.cend());
    //}

    template<size_t words_capacity>
    constexpr void BigInt<words_capacity>::UnsignedMultiplyByRange(const Range<Word> auto& range) noexcept {
        size_t range_words_count = RangeWordsCount(range);

        if (words_count == 1 || range_words_count == 1) {
            // Case when multiplication can be done linearly, without extra stack allocation for temporary result
            UnsignedShortMultiplyByRange(range);
        } else if constexpr (words_capacity <= 40) {
            //// for small numbers do quadratic multiplication
            BigInt ret;
            auto it = std::ranges::begin(range);
            for (size_t i = 0; i < range_words_count; ++i, ++it) {
                BigInt tmp {*this};
                tmp.UnsignedShortMultiplyByRange(std::ranges::single_view(*it));
                tmp <<= i * kWordBSize;
                ret.UnsignedAddRange(std::ranges::subrange(tmp.cbegin(), tmp.cend()));
            }
            UnsignedResetBinary(std::ranges::subrange(ret.cbegin(), ret.cend()));
        //} else {
            //// https://www.geeksforgeeks.org/karatsuba-algorithm-for-fast-multiplication-using-divide-and-conquer-algorithm/
            //if (size_t max_size = words_count + range_words_count; max_size <= words_capacity / 16) {
                //KaratsubaMultiplyByRange<words_capacity / 16 + 1>(begin, begin + range_words_count);
            //} else if (max_size <= words_capacity / 8) {
                //KaratsubaMultiplyByRange<words_capacity / 8 + 1>(begin, begin + range_words_count);
            //} else if (max_size <= words_capacity / 4) {
                //KaratsubaMultiplyByRange<words_capacity / 4 + 1>(begin, begin + range_words_count);
            //} else if (max_size <= words_capacity / 2) {
                //KaratsubaMultiplyByRange<words_capacity / 2 + 1>(begin, begin + range_words_count);
            //} else {
                //KaratsubaMultiplyByRange<words_capacity>(begin, begin + range_words_count);
            //}
        } else {
            ASSERT(false, "Unreachable");
        }
    }

    template<size_t words_capacity>
    constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator*=(const BigInt& rhs) noexcept {
        if (rhs.IsPowerOf2()) {
            return *this <<= rhs.BitWidth() - 1;
        }

        is_positive ^= !rhs.is_positive;
        UnsignedMultiplyByRange(std::ranges::subrange(rhs.cbegin(), rhs.cend()));
        return *this;
    }

    //template<size_t words_capacity>
    //constexpr BigInt<words_capacity> BigInt<words_capacity>::ShortUnsignedDivideBy(uint32_t rhs) noexcept {
        //BigInt remainder;
        //bool set = false;
        //const size_t iterations = words_count;
        //words_count = 0;
        //uint64_t window = 0;
        //for (size_t i = 0; i < iterations; ++i) {
            //window = (window << 32) + binary[iterations - 1 - i];
            //if (window >= rhs) {
                //binary[iterations - 1 - i] = window / rhs;
                //if (!set && binary[iterations - 1 - i]) {
                    //set = true;
                    //words_count = iterations - i;
                //}
                //window %= rhs;
            //} else {
                //binary[iterations - 1 - i] = 0;
            //}
        //}

        //if ((remainder.binary[0] = static_cast<uint32_t>(window))) {
            //remainder.words_count = 1;
        //}
        //return remainder;
    //}

    //template<size_t words_capacity>
    //template<IntIter It>
    //constexpr BigInt<words_capacity> BigInt<words_capacity>::UnsignedDivideByRange(It begin, It end) noexcept {
        //size_t range_words_count = RangeWordsCount(begin, end);
        //ASSERT(range_words_count <= words_count);

        //if (range_words_count == 1) {
            //return ShortUnsignedDivideBy(*begin);
        //}

        //auto unsigned_cmp = [](It lhs_begin, It lhs_end, It rhs_begin, It rhs_end) {
            //if (auto cmp = lhs_end - lhs_begin <=> rhs_end - rhs_begin; cmp != 0) {
                //return cmp;
            //}

            //It lhs_rend = lhs_end - 1;
            //It rhs_rend = rhs_end - 1;
            //while (lhs_rend != lhs_begin) {
                //if (auto cmp = *lhs_rend <=> *rhs_rend; cmp != 0) {
                    //return cmp;
                //}
                //--lhs_rend;
                //--rhs_rend;
            //}

            //return *lhs_begin <=> *rhs_begin;
        //};

        //if (words_count - range_words_count <= 1) {
            //// binary search
            //uint32_t l = 0;
            //uint32_t r = kMaxWord;
            //uint32_t m;
            //while (l != r) {
                //m = (l / 2) + (r / 2) + (l & r & 1);
                //BigInt<words_capacity + 1> tmp {m};
                //tmp.UnsignedMultiplyByRange(begin, begin + range_words_count);
                //if (auto cmp = unsigned_cmp(tmp.cbegin(), tmp.cend(), cbegin(), cend()); cmp == 0) {
                    //UnsignedResetBinary(&m, &m + 1);
                    //return 0;
                //} else if (cmp > 0) {
                    //r = m - 1;
                //} else {
                    //BigInt remainder = *this;
                    //remainder.UnsignedSubSmallerRange(tmp.cbegin(), tmp.cend());
                    //if (unsigned_cmp(remainder.cbegin(), remainder.cend(), begin, begin + range_words_count) < 0) {
                        //UnsignedResetBinary(&m, &m + 1);
                        //return remainder;
                    //} else {
                        //l = m + 1;
                    //}
                //}
            //}
            //m = l;
            //BigInt tmp {m};
            //tmp.UnsignedMultiplyByRange(begin, begin + range_words_count);
            //BigInt remainder = *this;
            //remainder.UnsignedSubSmallerRange(tmp.cbegin(), tmp.cend());
            //UnsignedResetBinary(&m, &m + 1);
            //return remainder;
        //} else {
            //const size_t old_words_count = words_count;
            //BigInt remainder;
            //bool set = false;
            //for (size_t i = 0; i < old_words_count; ++i) {
                //size_t idx = old_words_count - 1 - i;
                //remainder <<= 32;
                //remainder.UnsignedAddRange(cbegin() + idx, cbegin() + idx + 1);
                //if (unsigned_cmp(remainder.cbegin(), remainder.cend(), begin, begin + range_words_count) >= 0) {
                    //ASSERT(remainder.words_count - range_words_count <= 1);
                    //BigInt tmp = remainder;
                    //remainder = tmp.UnsignedDivideByRange(begin, begin + range_words_count);
                    //ASSERT(tmp.words_count <= 1);
                    //binary[idx] = tmp.words_count == 1 ? tmp.binary[0] : 0;
                    //if (!set) {
                        //words_count = idx + 1;
                        //set = true;
                    //}
                //} else {
                    //binary[idx] = 0;
                //}
            //}
            //return remainder;
        //}
    //}

    //template<size_t words_capacity>
    //constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator/=(const BigInt& rhs) noexcept {
        //ASSERT(!rhs.IsZero());
        //if (IsZero() || rhs.IsOne()) {
            //return *this;
        //} else if (words_count < rhs.words_count) {
            //return *this = 0;
        //}

        //is_positive ^= !rhs.is_positive;
        //if (rhs.IsPowerOf2()) {
            //*this >>= rhs.BitWidth() - 1;
        //} else {
            //UnsignedDivideByRange(rhs.cbegin(), rhs.cend());
        //}
        //return *this;
    //}

    //template<size_t words_capacity>
    //constexpr BigInt<words_capacity>& BigInt<words_capacity>::operator%=(const BigInt& rhs) noexcept {
        //ASSERT(!rhs.IsZero());
        //ASSERT(!rhs.IsOne() && rhs.is_positive);
        //if (IsZero() || words_count < rhs.words_count) {
            //return *this;
        //} else if (rhs.IsPowerOf2()) {
            //return *this &= rhs - 1;
        //}

        //BigInt remainder = UnsignedDivideByRange(rhs.cbegin(), rhs.cend());
        //UnsignedResetBinary(remainder.cbegin(), remainder.cend());
        //return *this;
    //}

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

    template<size_t _>
    constexpr const BigInt<_>::Word* BigInt<_>::cbegin() const noexcept {
        return binary.cbegin();
    }

    template<size_t _>
    constexpr const BigInt<_>::Word* BigInt<_>::cend() const noexcept {
        return binary.cbegin() + words_count;
    }

    template<size_t words_capacity>
    constexpr size_t BigInt<words_capacity>::RangeWordsCount(const Range<Word> auto& range) noexcept {
        if constexpr (std::ranges::random_access_range<decltype(range)>) {
            size_t size = std::ranges::size(range);
            auto data = std::ranges::begin(range);
            for (size_t i = 0; i < size; ++i) {
                if (data[size - 1 - i]) {
                    ASSERT(size - i <= words_capacity, "Range is too big");
                    return size - i;
                }
            }
            return 0;
        } else {
            size_t counter = 1;
            size_t ret = 0;
            for (auto it = std::ranges::begin(range); it != std::ranges::end(range); ++it) {
                if (*it != 0) {
                    ret = counter;
                }
                counter += 1;
            }
            ASSERT(ret <= words_capacity, "Range is too big");
            return ret;
        }
    }

    template<size_t _>
    constexpr bool BigInt<_>::operator==(const BigInt& rhs) const noexcept {
        return (*this <=> rhs) == 0;
    }

    template<size_t _>
    constexpr auto BigInt<_>::operator<=>(const BigInt& rhs) const noexcept {
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
    template<size_t _>
    constexpr BigInt<_> operator+(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs += rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator+(const BigInt<_>& lhs, T&& rhs) noexcept {
        return BigInt<_>{std::forward<T>(rhs)} + lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator+(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} + rhs;
    }

    template<size_t _>
    constexpr BigInt<_> operator-(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs -= rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator-(const BigInt<_>& lhs, T&& rhs) noexcept {
        return -(BigInt<_>{std::forward<T>(rhs)} - lhs);
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator-(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} - rhs;
    }

    template<size_t _>
    constexpr BigInt<_> operator*(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator*(const BigInt<_>& lhs, T&& rhs) noexcept {
        return BigInt<_>{std::forward<T>(rhs)} * lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator*(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} * rhs;
    }

    template<size_t _>
    constexpr BigInt<_> operator/(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs /= rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator/(BigInt<_> lhs, T&& rhs) noexcept {
        lhs /= BigInt<_>{std::forward<T>(rhs)};
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator/(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} / rhs;
    }

    template<size_t _>
    constexpr BigInt<_> operator%(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs %= rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator%(BigInt<_> lhs, T&& rhs) noexcept {
        lhs %= BigInt<_>{std::forward<T>(rhs)};
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator%(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} % rhs;
    }

    template<size_t _>
    constexpr BigInt<_> operator&(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs &= rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator&(const BigInt<_>& lhs, T&& rhs) noexcept {
        return BigInt<_>{std::forward<T>(rhs)} & lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator&(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} & rhs;
    }

    template<size_t _>
    constexpr BigInt<_> operator|(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs |= rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator|(const BigInt<_>& lhs, T&& rhs) noexcept {
        return BigInt<_>{std::forward<T>(rhs)} | lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator|(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} | rhs;
    }

    template<size_t _>
    constexpr BigInt<_> operator^(BigInt<_> lhs, const BigInt<_>& rhs) noexcept {
        lhs ^= rhs;
        return lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator^(const BigInt<_>& lhs, T&& rhs) noexcept {
        return BigInt<_>{std::forward<T>(rhs)} ^ lhs;
    }

    template<size_t _, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<_> operator^(T&& lhs, const BigInt<_>& rhs) noexcept {
        return BigInt<_>{std::forward<T>(lhs)} ^ rhs;
    }

} // namespace algo
