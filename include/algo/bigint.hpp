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
#include <vector>

namespace algo {

    template<size_t words_capacity, typename Word = uint32_t, typename DoubleWord = uint64_t>
    class BigInt {
        static_assert(words_capacity > 0 && !std::numeric_limits<Word>::is_signed && std::numeric_limits<Word>::is_modulo);
        static_assert(sizeof(DoubleWord) >= sizeof(Word) * 2, "DoubleWord should be able to hold result of any Word multiplication");

        static constexpr size_t kWordBSize = std::numeric_limits<Word>::digits;
        static_assert(std::numeric_limits<size_t>::max() / kWordBSize >= words_capacity, "Every single bit should be indexable by size_t");

        static constexpr Word kMaxWord = std::numeric_limits<Word>::max();

    public:
        using Storage = std::conditional_t<
                            words_capacity == -1ull && false, // TODO add support for vector
                            std::vector<Word>,
                            std::array<Word, words_capacity>
                        >;

        constexpr BigInt() noexcept = default;
        constexpr BigInt(DoubleWord integer, bool is_positive = true) noexcept;
        constexpr BigInt(std::string_view str) noexcept;

        constexpr BigInt(const Range<Word> auto& range, bool is_positive = true) noexcept;

        constexpr bool operator==(const BigInt&) const noexcept;
        constexpr std::strong_ordering operator<=>(const BigInt&) const noexcept;

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
        constexpr bool IsPowerOf2() const noexcept;
        constexpr size_t BitWidth() const noexcept;

        Storage binary = {}; // number is storred right to left, e.g. most significant bits are at the end of an array
                             // can't use bitset here, because not constexpr (since C++23)
        size_t words_count = 0;
        bool is_positive = true;

    private:
        template<size_t s, typename w, typename dw>
        friend class BigInt;

        static constexpr size_t RangeWordsCount(const Range<Word> auto& range) noexcept; 

        constexpr auto ToView() const noexcept;
        constexpr std::strong_ordering UnsignedCompare(const RandomAccessRange<Word> auto& range) const noexcept;

        // All operations below can work properly if range points to subrange of this->binary
        constexpr void UnsignedResetBinary(const Range<Word> auto& range) noexcept;
        constexpr void UnsignedAddRange(const Range<Word> auto& range) noexcept;
        constexpr bool UnsignedSubRange(const Range<Word> auto& range) noexcept; // subtract by range and return if sign changes
        constexpr void UnsignedSubSmallerRange(const Range<Word> auto& range) noexcept;
        constexpr void UnsignedShortMultiplyByRange(const Range<Word> auto& range) noexcept; // either this or rhs is lesser than Word
        template<size_t subint_words_capacity>
        constexpr void KaratsubaMultiplyByRange(const Range<Word> auto& range) noexcept;
        constexpr void UnsignedMultiplyByRange(const Range<Word> auto& range) noexcept;
        constexpr BigInt ShortUnsignedDivideBy(Word) noexcept; // returns remainder
        constexpr BigInt UnsignedDivideByRange(const Range<Word> auto& range) noexcept; // returns remainder

        constexpr void PowerInner(BigInt& res, BigInt&& exp) const noexcept;
    };

    // Traits
    using VecBigInt = BigInt<-1ull>;

    template<typename T>
    struct IsBigInt : std::false_type {};

    template<size_t S, typename Word, typename DoubleWord>
    struct IsBigInt<BigInt<S, Word, DoubleWord>> : std::true_type {};

    // Implementation
    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>::BigInt(DoubleWord from, bool is_positive) noexcept {
        ASSERT(words_capacity > 1 || from <= kMaxWord, "Given integer won't fit in provided type");

        if (from == 0) {
            return;
        }

        binary[0] = from & kMaxWord;
        if (from > kMaxWord) {
            binary[1] = from >> kWordBSize;
            words_count = 2;
        } else {
            words_count = 1;
        }
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>::BigInt(std::string_view str) noexcept {
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>::BigInt(const Range<Word> auto& range, bool is_positive) noexcept
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>& BigInt<words_capacity, Word, DoubleWord>::operator<<=(size_t shift) noexcept {
        ASSERT(shift < words_capacity * kWordBSize, "Shift is bigger than bit size");

        if (shift == 0) {
            return *this;
        }

        size_t word_offset = shift / kWordBSize;
        size_t bit_offset = shift % kWordBSize;
        auto get_shifted_word = [&](size_t idx) -> Word {
            Word ret = 0;
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>& BigInt<words_capacity, Word, DoubleWord>::operator>>=(size_t shift) noexcept {
        ASSERT(shift < words_capacity * kWordBSize, "Shift is bigger than bit size");
        if (shift == 0) {
            return *this;
        }
        size_t word_offset = shift / kWordBSize;
        size_t bit_offset = shift % kWordBSize;
        auto get_shifted_word = [&](size_t idx) -> Word {
            Word ret = 0;
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord> BigInt<words_capacity, Word, DoubleWord>::operator<<(size_t shift_int) const noexcept {
        return BigInt{*this} <<= shift_int;
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord> BigInt<words_capacity, Word, DoubleWord>::operator>>(size_t shift_int) const noexcept {
        return BigInt{*this} >>= shift_int;
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord> BigInt<words_capacity, Word, DoubleWord>::operator-() const noexcept {
        BigInt copy = *this;
        copy.is_positive ^= true;
        return copy;
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr void BigInt<words_capacity, Word, DoubleWord>::UnsignedResetBinary(const Range<Word> auto& range) noexcept {
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr void BigInt<words_capacity, Word, DoubleWord>::UnsignedAddRange(const Range<Word> auto& range) noexcept {
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>& BigInt<words_capacity, Word, DoubleWord>::operator+=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            is_positive ^= true;
            *this -= rhs;
            is_positive ^= true;
            return *this;
        } else {
            UnsignedAddRange(rhs.ToView());
            return *this;
        }
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr bool BigInt<words_capacity, Word, DoubleWord>::UnsignedSubRange(const Range<Word> auto& range) noexcept {
        bool this_greater_equal = UnsignedCompare(range);
        Word borrow = 0;
        size_t new_words_count = 0;
        size_t i = 0;
        auto it = std::ranges::begin(range);
        while (i < words_capacity) {
            Word lhs {0}, rhs {0};
            if (i < words_count) {
                lhs = binary[i];
            }
            if (it != std::ranges::end(range)) {
                rhs = *it;
                ++it;
            }
            if (!this_greater_equal) {
                std::swap(lhs, rhs);
            }
            if (rhs != kMaxWord || borrow == 0) {
                Word tmp = rhs + borrow;
                if (lhs < tmp) {
                    borrow = 1;
                } else {
                    borrow = 0;
                }
                binary[i] = lhs - tmp;
                if (binary[i]) {
                    new_words_count = i + 1;
                }
            }
            ++i;
        }
        words_count = new_words_count;
        return !this_greater_equal;
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr void BigInt<words_capacity, Word, DoubleWord>::UnsignedSubSmallerRange(const Range<Word> auto& range) noexcept {
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>& BigInt<words_capacity, Word, DoubleWord>::operator-=(const BigInt& rhs) noexcept {
        if (is_positive ^ rhs.is_positive) {
            UnsignedAddRange(rhs.ToView());
            return *this;
        }

        if (auto cmp = *this <=> rhs; cmp == 0) {
            return *this = {};
        } else if ((cmp > 0 && !is_positive) || (cmp < 0 && is_positive)) { // TODO Make subtraction for any range
            return *this = -(rhs - *this);
        }

        UnsignedSubSmallerRange(rhs.ToView());
        return *this;
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr void BigInt<words_capacity, Word, DoubleWord>::UnsignedShortMultiplyByRange(const Range<Word> auto& range) noexcept {
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

    template<size_t words_capacity, typename Word, typename DoubleWord>
    template<size_t subint_words_capacity>
    constexpr void BigInt<words_capacity, Word, DoubleWord>::KaratsubaMultiplyByRange(const Range<Word> auto& range) noexcept {
        // https://www.geeksforgeeks.org/karatsuba-algorithm-for-fast-multiplication-using-divide-and-conquer-algorithm/
        using SmallInt = BigInt<subint_words_capacity>; // Multiplication result can fit into SmallInt

        const size_t mid_thr = (std::max(RangeWordsCount(range), words_count) + 1) / 2;

        SmallInt this_l {std::ranges::take_view(ToView(), mid_thr)};
        SmallInt this_h {std::ranges::drop_view(ToView(), mid_thr)};

        SmallInt rhs_l {std::ranges::take_view(range, mid_thr)};
        SmallInt rhs_h {std::ranges::drop_view(range, mid_thr)};

        // 2^32 * (2^32 * (this_h * rhs_h) + (this_h + this_l) * (rhs_h * rhs_l) - this_h * rhs_h - this_l * rhs_l) + this_l * rhs_l
        SmallInt mix = (this_l + this_h) * (rhs_l + rhs_h);
        this_h *= rhs_h;
        this_l *= rhs_l;

        UnsignedResetBinary(this_h.ToView());
        *this <<= mid_thr * kWordBSize;
        UnsignedAddRange(mix.ToView());
        UnsignedSubSmallerRange(this_h.ToView());
        UnsignedSubSmallerRange(this_l.ToView());
        *this <<= mid_thr * kWordBSize;
        UnsignedAddRange(this_l.ToView());
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr void BigInt<words_capacity, Word, DoubleWord>::UnsignedMultiplyByRange(const Range<Word> auto& range) noexcept {
        size_t range_words_count = RangeWordsCount(range);

        if (words_count == 1 || range_words_count == 1) {
            // Case when multiplication can be done linearly, without extra stack allocation for temporary result
            UnsignedShortMultiplyByRange(range);
        } else if constexpr (words_capacity <= 40) {
            //// for small numbers do quadratic multiplication
            BigInt ret;
            auto it = std::ranges::begin(range);
            for (size_t i = 0; i < range_words_count; ++i, ++it) {
                // Try to perform multiplication on smaller type because we don't need to allocate whole array
#define TRY_OPTIMIZE(cap) if (words_count + i + 1 < cap) { \
                              BigInt<cap> tmp {ToView()}; \
                              tmp.UnsignedShortMultiplyByRange(std::ranges::single_view(*it)); \
                              tmp <<= i * kWordBSize; \
                              ret.UnsignedAddRange(tmp.ToView()); \
                          }

                TRY_OPTIMIZE(words_capacity / 16 + 1)
                else TRY_OPTIMIZE(words_capacity / 8 + 1)
                else TRY_OPTIMIZE(words_capacity / 4 + 1)
                else TRY_OPTIMIZE(words_capacity / 2 + 1)
                else {
                    BigInt tmp {*this};
                    tmp.UnsignedShortMultiplyByRange(std::ranges::single_view(*it));
                    tmp <<= i * kWordBSize;
                    ret.UnsignedAddRange(tmp.ToView());
                }
#undef TRY_OPTIMIZE
            }
            UnsignedResetBinary(ret.ToView());
        } else {
            size_t max_size = words_count + range_words_count;
            // If result fits into smaller array, then perform multiplication using smaller array
#define TRY_OPTIMIZE(cap) if (max_size < cap) { KaratsubaMultiplyByRange<cap>(range); return; }
            TRY_OPTIMIZE(words_capacity / 16 + 1)
            TRY_OPTIMIZE(words_capacity / 8 + 1)
            TRY_OPTIMIZE(words_capacity / 4 + 1)
            TRY_OPTIMIZE(words_capacity / 2 + 1)
#undef TRY_OPTIMIZE
            KaratsubaMultiplyByRange<words_capacity, Word, DoubleWord>(range);
        }
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord>& BigInt<words_capacity, Word, DoubleWord>::operator*=(const BigInt& rhs) noexcept {
        if (rhs.IsPowerOf2()) {
            return *this <<= rhs.BitWidth() - 1;
        }

        is_positive ^= !rhs.is_positive;
        UnsignedMultiplyByRange(rhs.ToView());
        return *this;
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr BigInt<words_capacity, Word, DoubleWord> BigInt<words_capacity, Word, DoubleWord>::ShortUnsignedDivideBy(Word rhs) noexcept {
        BigInt remainder;
        bool set = false;
        const size_t iterations = words_count;
        words_count = 0;
        DoubleWord window = 0;
        for (size_t i = 0; i < iterations; ++i) {
            window = (window << kWordBSize) + binary[iterations - 1 - i];
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

        if ((remainder.binary[0] = static_cast<Word>(window))) {
            remainder.words_count = 1;
        }
        return remainder;
    }

    //template<size_t words_capacity, typename Word, typename DoubleWord>
    //template<IntIter It>
    //constexpr BigInt<words_capacity, Word, DoubleWord> BigInt<words_capacity, Word, DoubleWord>::UnsignedDivideByRange(It begin, It end) noexcept {
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

    //template<size_t words_capacity, typename Word, typename DoubleWord>
    //constexpr BigInt<words_capacity, Word, DoubleWord>& BigInt<words_capacity, Word, DoubleWord>::operator/=(const BigInt& rhs) noexcept {
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

    //template<size_t words_capacity, typename Word, typename DoubleWord>
    //constexpr BigInt<words_capacity, Word, DoubleWord>& BigInt<words_capacity, Word, DoubleWord>::operator%=(const BigInt& rhs) noexcept {
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

    template<size_t words_capacity, typename W, typename DW>
    constexpr BigInt<words_capacity, W, DW> BigInt<words_capacity, W, DW>::operator~() const noexcept {
        static_assert(words_capacity != -1ull, "Can't negate unbound BigInt");
        BigInt ret;
        ret.is_positive = is_positive;
        for (size_t i = 0; i < words_capacity; ++i) {
            if ((ret.binary[i] = ~binary[i])) {
                ret.words_count = i + 1;
            }
        }
        return ret;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW>& BigInt<c, W, DW>::operator^=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if ((binary[i] ^= other.binary[i])) {
                words_count = i + 1;
            }
        }
        return *this;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW>& BigInt<c, W, DW>::operator|=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = iterations;
        for (size_t i = 0; i < iterations; ++i) {
            binary[i] |= other.binary[i];
        }
        return *this;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW>& BigInt<c, W, DW>::operator&=(const BigInt& other) noexcept {
        const size_t iterations = std::max(words_count, other.words_count);
        words_count = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if ((binary[i] &= other.binary[i])) {
                words_count = i + 1;
            }
        }
        return *this;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> BigInt<c, W, DW>::Power(BigInt exp) const noexcept {
        BigInt ret;
        PowerInner(ret, std::move(exp));
        return ret;
    }

    template<size_t c, typename W, typename DW>
    constexpr void BigInt<c, W, DW>::PowerInner(BigInt& res, BigInt&& exp) const noexcept {
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

    template<size_t c, typename W, typename DW>
    constexpr bool BigInt<c, W, DW>::IsZero() const noexcept {
        return words_count == 0;
    }

    template<size_t c, typename W, typename DW>
    constexpr bool BigInt<c, W, DW>::IsPowerOf2() const noexcept {
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

    template<size_t c, typename W, typename DW>
    constexpr size_t BigInt<c, W, DW>::BitWidth() const noexcept {
        if (IsZero()) {
            return 0;
        }
        return (words_count - 1) * 32 + std::bit_width(binary[words_count - 1]);
    }

    template<size_t c, typename W, typename DW>
    constexpr auto BigInt<c, W, DW>::ToView() const noexcept {
        return std::views::counted(binary.cbegin(), words_count);
    }

    template<size_t words_capacity, typename Word, typename DoubleWord>
    constexpr size_t BigInt<words_capacity, Word, DoubleWord>::RangeWordsCount(const Range<Word> auto& range) noexcept {
        if constexpr (std::ranges::random_access_range<decltype(range)>) {
            size_t size = std::ranges::size(range);
            ASSERT(size <= words_capacity, "Range is too big");
            auto data = std::ranges::begin(range);
            for (size_t i = 0; i < size; ++i) {
                if (data[size - 1 - i]) {
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

    template<size_t c, typename W, typename DW>
    constexpr std::strong_ordering BigInt<c, W, DW>::UnsignedCompare(const RandomAccessRange<W> auto& range) const noexcept {
        size_t range_words_count = RangeWordsCount(range);

        if (words_count < range_words_count) {
            return std::strong_ordering::less;
        } else if (words_count > range_words_count) {
            return std::strong_ordering::greater;
        }

        auto range_data = std::ranges::begin(range);
        for (size_t i = 0; i < words_count; ++i) {
            if (auto cmp = binary[words_count - 1 - i] <=> range_data[words_count - 1 - i]; cmp != 0) {
                return cmp;
            }
        }
        return std::strong_ordering::equal;
    }

    template<size_t c, typename W, typename DW>
    constexpr bool BigInt<c, W, DW>::operator==(const BigInt& rhs) const noexcept {
        return (*this <=> rhs) == 0;
    }

    template<size_t c, typename W, typename DW>
    constexpr std::strong_ordering BigInt<c, W, DW>::operator<=>(const BigInt& rhs) const noexcept {
        if (IsZero() && rhs.IsZero()) {
            return std::strong_ordering::equal;
        } else if (is_positive ^ rhs.is_positive) {
            return is_positive <=> rhs.is_positive;
        } else if (auto cmp = UnsignedCompare(rhs.ToView()); is_positive) {
            return cmp;
        } else {
            return 0 <=> cmp;
        }
    }

    // Arithmetic opeartors
    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator+(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs += rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator+(const BigInt<c, W, DW>& lhs, T&& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(rhs)} + lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator+(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} + rhs;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator-(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs -= rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator-(const BigInt<c, W, DW>& lhs, T&& rhs) noexcept {
        return -(BigInt<c, W, DW>{std::forward<T>(rhs)} - lhs);
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator-(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} - rhs;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator*(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator*(const BigInt<c, W, DW>& lhs, T&& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(rhs)} * lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator*(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} * rhs;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator/(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs /= rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator/(BigInt<c, W, DW> lhs, T&& rhs) noexcept {
        lhs /= BigInt<c, W, DW>{std::forward<T>(rhs)};
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator/(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} / rhs;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator%(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs %= rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator%(BigInt<c, W, DW> lhs, T&& rhs) noexcept {
        lhs %= BigInt<c, W, DW>{std::forward<T>(rhs)};
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator%(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} % rhs;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator&(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs &= rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator&(const BigInt<c, W, DW>& lhs, T&& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(rhs)} & lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator&(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} & rhs;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator|(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs |= rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator|(const BigInt<c, W, DW>& lhs, T&& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(rhs)} | lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator|(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} | rhs;
    }

    template<size_t c, typename W, typename DW>
    constexpr BigInt<c, W, DW> operator^(BigInt<c, W, DW> lhs, const BigInt<c, W, DW>& rhs) noexcept {
        lhs ^= rhs;
        return lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator^(const BigInt<c, W, DW>& lhs, T&& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(rhs)} ^ lhs;
    }

    template<size_t c, typename W, typename DW, typename T>
        requires (!IsBigInt<std::remove_cvref_t<T>>::value)
    constexpr BigInt<c, W, DW> operator^(T&& lhs, const BigInt<c, W, DW>& rhs) noexcept {
        return BigInt<c, W, DW>{std::forward<T>(lhs)} ^ rhs;
    }

} // namespace algo
