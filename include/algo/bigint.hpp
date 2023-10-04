#pragma once

#include <algo/assert.hpp>
#include <algo/concepts.hpp>

#include <algorithm>
#include <bit>
#include <cassert>
#include <limits>
#include <vector>

namespace algo {

template<std::size_t words_capacity, typename Word = uint32_t,
         typename DoubleWord = uint64_t>
class BigInt {
  static_assert(words_capacity > 0 && !std::numeric_limits<Word>::is_signed &&
                std::numeric_limits<Word>::is_modulo &&
                !std::numeric_limits<DoubleWord>::is_signed &&
                std::numeric_limits<DoubleWord>::is_modulo);

  static_assert(sizeof(DoubleWord) >= sizeof(Word) * 2,
                "DoubleWord should be able to hold "
                "result of any Word multiplication");

  static constexpr bool kInfInt =
      (words_capacity == std::numeric_limits<std::size_t>::max());

  static constexpr std::size_t kWordBSize = std::numeric_limits<Word>::digits;

  static_assert(kInfInt ||
                    std::numeric_limits<std::size_t>::max() / kWordBSize >=
                        words_capacity,
                "Every single bit should be indexable by std::size_t");

  static constexpr Word kMaxWord = std::numeric_limits<Word>::max();

public:
  constexpr BigInt() noexcept;
  constexpr BigInt(const BigInt&) noexcept;
  constexpr BigInt(BigInt&&) noexcept;

  constexpr BigInt& operator=(const BigInt&) noexcept;
  constexpr BigInt& operator=(BigInt&&) noexcept;

  constexpr BigInt(uint64_t integer, bool is_positive = true) noexcept;
  constexpr BigInt(std::string_view str) noexcept;

  constexpr BigInt(const Range<Word> auto& range,
                   bool is_positive = true) noexcept;

  constexpr bool operator==(const BigInt&) const noexcept;
  constexpr std::strong_ordering operator<=>(const BigInt&) const noexcept;

  constexpr BigInt& operator<<=(std::size_t) noexcept;
  constexpr BigInt& operator>>=(std::size_t) noexcept;
  constexpr BigInt operator<<(std::size_t) const noexcept;
  constexpr BigInt operator>>(std::size_t) const noexcept;

  constexpr const BigInt& operator+() const noexcept;
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

  constexpr bool IsZero() const noexcept;
  constexpr bool IsPowerOf2() const noexcept;
  constexpr std::size_t BitWidth() const noexcept;
  constexpr std::string ToString(Word base = 10) const noexcept;
  constexpr uint64_t ToUint() const noexcept;
  constexpr auto ToView() const noexcept;

  friend std::ostream& operator<<(std::ostream& os, const BigInt& bi) {
    os << bi.ToString();
    return os;
  }

  std::conditional_t<kInfInt, // TODO add support for vector
                     std::vector<Word>, std::array<Word, words_capacity>>
      binary; // number is storred right to left, e.g. most significant bits
              // are at the end of an array
              // can't use bitset here, because not constexpr (since C++23)
  std::size_t words_count;
  bool is_positive;

private:
  template<std::size_t s, typename W, typename DW>
  friend class BigInt;

  // All operations below can work properly if
  // range points to subrange of this->binary
  static constexpr bool
  RangeIsZero(const RandomAccessRange<Word> auto& range) noexcept;
  static constexpr std::size_t
  RangeBitWidth(const RandomAccessRange<Word> auto& range) noexcept;

  constexpr std::strong_ordering
  UCompare(const RandomAccessRange<Word> auto& range) const noexcept;

  constexpr void
  UResetBinary(const RandomAccessRange<Word> auto& range) noexcept;
  constexpr void UAddRange(const RandomAccessRange<Word> auto& range) noexcept;

  // subtract by range and return if sign changes
  constexpr bool USubRange(const RandomAccessRange<Word> auto& range) noexcept;

  // Multiplication
  // either this or rhs is lesser than Word
  constexpr void
  UMulByShortRange(const RandomAccessRange<Word> auto& range) noexcept;

  template<std::size_t subint_words_capacity>
  constexpr void
  KaratsubaUMulByRange(const RandomAccessRange<Word> auto& range) noexcept;

  constexpr void
  UMulByRange(const RandomAccessRange<Word> auto& range) noexcept;

  // Division
  constexpr Word UDivByWord(Word rhs) noexcept; // returns remainder
  // result of division will fit into Word type
  constexpr BigInt
  UDivBySameRange(const RandomAccessRange<Word> auto& range) noexcept;
  // returns remainder
  constexpr BigInt
  UDivByRange(const RandomAccessRange<Word> auto& range) noexcept;

  constexpr void PowerInner(BigInt& res, BigInt&& exp) const noexcept;
};

// Traits
using InfInt = BigInt<-1ull>;

template<typename T>
struct IsBigInt : std::false_type {};

template<std::size_t S, typename Word, typename DoubleWord>
struct IsBigInt<BigInt<S, Word, DoubleWord>> : std::true_type {};

// Implementation
template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>::BigInt() noexcept
    : words_count{1}
    , is_positive{true} {
  binary[0] = 0;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>::BigInt(const BigInt& other) noexcept
    : words_count{other.words_count}
    , is_positive{other.is_positive} {
  for (std::size_t i = 0; i < words_count; ++i) {
    binary[i] = other.binary[i];
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>::BigInt(BigInt&& other) noexcept
    : words_count{other.words_count}
    , is_positive{other.is_positive} {
  for (std::size_t i = 0; i < words_count; ++i) {
    binary[i] = other.binary[i];
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator=(const BigInt& other) noexcept {
  words_count = other.words_count;
  is_positive = other.is_positive;
  for (std::size_t i = 0; i < words_count; ++i) {
    binary[i] = other.binary[i];
  }
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator=(BigInt&& other) noexcept {
  words_count = other.words_count;
  is_positive = other.is_positive;
  for (std::size_t i = 0; i < words_count; ++i) {
    binary[i] = other.binary[i];
  }
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>::BigInt(uint64_t from,
                                     bool from_is_positive) noexcept
    : BigInt{} {
  is_positive = from_is_positive;
  if (from == 0) {
    return;
  }

  std::size_t idx = 0;
  while (from != 0) {
    assert(idx < cap && "Given integer won't fit in provided type");
    binary[idx] = kMaxWord & from;
    from >>= kWordBSize;
    ++idx;
  }
  words_count = idx;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>::BigInt(std::string_view str) noexcept
    : BigInt{} {
  ASSERT(str.size() != 0);

  if (str[0] == '-') {
    is_positive = false;
    str.remove_prefix(1);
  }

  W base;
  if (str.size() >= 2 && str.substr(0, 2) == "0b") {
    base = 2;
    str.remove_prefix(2);
  } else {
    base = 10;
  }

  ASSERT(str.size() != 0);
  for (char c : str) {
    if (c == '\'') {
      continue;
    }

    UMulByShortRange(std::ranges::single_view(base));
    UAddRange(std::ranges::single_view(static_cast<W>(c - '0')));
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>::BigInt(const Range<W> auto& range,
                                     bool is_positive) noexcept
    : words_count{1}
    , is_positive{is_positive} {
  binary[0] = 0;
  std::size_t counter = 1;
  for (auto it = std::ranges::begin(range); it != std::ranges::end(range);
       ++it) {
    ASSERT(counter <= cap, "Type is too small for provided range");
    if (*it > 0) {
      words_count = counter;
    }
    binary[counter - 1] = *it;

    ++counter;
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator<<=(std::size_t shift) noexcept {
  ASSERT(shift < cap * kWordBSize, "Shift is bigger than bit size");

  if (shift == 0 || IsZero()) {
    return *this;
  }

  std::size_t word_offset = shift / kWordBSize;
  std::size_t bit_offset = shift % kWordBSize;
  auto get_shifted_word = [&](std::size_t idx) -> W {
    W ret = 0;
    if (idx >= word_offset) {
      ret |= binary[idx - word_offset] << bit_offset;
      if (bit_offset != 0 && idx > word_offset) {
        ret |= binary[idx - word_offset - 1] >> (kWordBSize - bit_offset);
      }
    }
    return ret;
  };

  std::size_t max = std::min(words_count + word_offset + 1, cap);

  if (words_count < cap) {
    binary[words_count] = 0;
  }

  words_count = 1;
  for (std::size_t i = 0; i < max; ++i) {
    std::size_t idx = max - 1 - i;
    binary[idx] = get_shifted_word(idx);

    if (words_count == 1 && binary[idx] != 0) {
      words_count = idx + 1;
    }
  }

  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator>>=(std::size_t shift) noexcept {
  ASSERT(shift < cap * kWordBSize, "Shift is bigger than bit size");

  if (shift == 0 || IsZero()) {
    return *this;
  }

  std::size_t word_offset = shift / kWordBSize;
  std::size_t bit_offset = shift % kWordBSize;

  if (word_offset >= words_count) {
    words_count = 1;
    binary[0] = 0;
    return *this;
  }

  auto get_shifted_word = [&](std::size_t idx) -> W {
    W ret = 0;
    if (idx + word_offset < words_count) {
      ret |= binary[idx + word_offset] >> bit_offset;
    }

    if (bit_offset != 0 && idx + word_offset + 1 < words_count) {
      ret |= binary[idx + word_offset + 1] << (kWordBSize - bit_offset);
    }

    return ret;
  };

  std::size_t max = words_count - word_offset; // max >= 1
  for (std::size_t i = 0; i < max; ++i) {
    binary[i] = get_shifted_word(i);
  }

  if (max == 1) {
    words_count = 1;
  } else if (binary[max - 1] != 0) {
    words_count = max;
  } else {
    words_count = max - 1;
  }
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>
BigInt<cap, W, DW>::operator<<(std::size_t shift_int) const noexcept {
  return BigInt{*this} <<= shift_int;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>
BigInt<cap, W, DW>::operator>>(std::size_t shift_int) const noexcept {
  return BigInt{*this} >>= shift_int;
}

template<std::size_t cap, typename W, typename DW>
constexpr const BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator+() const noexcept {
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> BigInt<cap, W, DW>::operator-() const noexcept {
  BigInt copy = *this;
  copy.is_positive ^= true;
  return copy;
}

template<std::size_t cap, typename W, typename DW>
constexpr void BigInt<cap, W, DW>::UResetBinary(
    const RandomAccessRange<W> auto& range) noexcept {
  words_count = std::ranges::size(range);
  if (words_count == 0) {
    words_count = 1;
    binary[0] = 0;
  } else {
    ASSERT(words_count <= cap);
    auto range_data = std::ranges::begin(range);
    for (std::size_t i = 0; i < words_count; ++i) {
      binary[i] = range_data[i];
    }
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr void
BigInt<cap, W, DW>::UAddRange(const RandomAccessRange<W> auto& range) noexcept {
  auto range_data = std::ranges::begin(range);
  std::size_t range_wc = std::ranges::size(range);
  std::size_t i = 0;
  for (bool carry = false; carry || i < range_wc; ++i) {
    ASSERT(i < cap, "Addition overflow");

    W lhs{0}, rhs{0};
    if (i < words_count) {
      lhs = binary[i];
    }
    if (i < range_wc) {
      rhs = range_data[i];
    }

    if (rhs != kMaxWord || !carry) {
      if (carry) {
        rhs += 1;
      }

      if (kMaxWord - lhs < rhs) {
        carry = true;
      } else {
        carry = false;
      }

      binary[i] = lhs + rhs;
    } else if (i >= words_count) {
      binary[i] = 0;
    }
  }

  if (i > words_count) {
    words_count = i;
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr bool
BigInt<cap, W, DW>::USubRange(const RandomAccessRange<W> auto& range) noexcept {
  auto lhs_data = std::ranges::begin(ToView());
  std::size_t lhs_wc = words_count;

  auto rhs_data = std::ranges::begin(range);
  std::size_t rhs_wc = std::ranges::size(range);

  bool this_ge = UCompare(range) >= 0;
  if (!this_ge) {
    std::swap(lhs_data, rhs_data);
    std::swap(lhs_wc, rhs_wc);
  }

  words_count = 1;
  std::size_t i = 0;
  for (bool borrow = false; borrow || i < rhs_wc; ++i) {
    W lhs{0}, rhs{0};
    if (i < lhs_wc) {
      lhs = lhs_data[i];
    }
    if (i < rhs_wc) {
      rhs = rhs_data[i];
    }

    binary[i] = lhs;
    if (rhs != kMaxWord || !borrow) {
      if (borrow) {
        rhs += 1;
      }

      if (lhs < rhs) {
        borrow = true;
      } else {
        borrow = false;
      }

      binary[i] -= rhs;
    }

    if (binary[i] != 0) {
      words_count = i + 1;
    }
  }

  if (i < lhs_wc) {
    words_count = lhs_wc;
  }

  for (; i < words_count; ++i) {
    binary[i] = lhs_data[i];
  }

  return !this_ge;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator+=(const BigInt& rhs) noexcept {
  if (is_positive ^ rhs.is_positive) {
    is_positive ^= USubRange(rhs.ToView());
  } else {
    UAddRange(rhs.ToView());
  }
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator-=(const BigInt& rhs) noexcept {
  if (is_positive ^ rhs.is_positive) {
    UAddRange(rhs.ToView());
  } else {
    is_positive ^= USubRange(rhs.ToView());
  }
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr void BigInt<cap, W, DW>::UMulByShortRange(
    const RandomAccessRange<W> auto& range) noexcept {
  if (IsZero()) {
    return;
  } else if (RangeIsZero(range)) {
    words_count = 1;
    binary[0] = 0;
    return;
  }

  std::size_t range_wc = std::ranges::size(range);

  ASSERT(words_count == 1 || range_wc == 1,
         "Short multiplication not applicable");

  auto range_data = std::ranges::begin(range);
  std::size_t i = 0;
  W lhs{binary[0]}, rhs{range_data[0]};
  binary[0] = 0;
  for (; i < words_count || i < range_wc; ++i) {
    DW prod = static_cast<DW>(lhs) * rhs;
    if (i + 1 < words_count) {
      lhs = binary[i + 1];
    } else if (i + 1 < range_wc) {
      rhs = range_data[i + 1];
    }

    W prod_l = prod;
    W prod_h = prod >> kWordBSize;

    if (i + 1 < cap) {
      binary[i + 1] = prod_h;
    } else {
      ASSERT(prod_h == 0, "Multiplication overflow");
    }

    if (kMaxWord - prod_l < binary[i]) {
      ASSERT(i + 1 < cap, "Multiplication overflow");
      binary[i + 1] += 1;
    }

    binary[i] += prod_l;
  }

  if (i < cap && binary[i] != 0) {
    words_count = i + 1;
  } else {
    words_count = i;
  }
}

template<std::size_t cap, typename W, typename DW>
template<std::size_t subint_cap>
constexpr void BigInt<cap, W, DW>::KaratsubaUMulByRange(
    const RandomAccessRange<W> auto& range) noexcept {
  // Multiplication result fits into SmallInt
  using SmallInt = BigInt<subint_cap, W, DW>;

  const std::size_t mid_thr =
      (std::max(std::ranges::size(range), words_count) + 1) / 2;

  SmallInt this_l{std::ranges::take_view(ToView(), mid_thr)};
  SmallInt this_h{std::ranges::drop_view(ToView(), mid_thr)};

  SmallInt rhs_l{std::ranges::take_view(range, mid_thr)};
  SmallInt rhs_h{std::ranges::drop_view(range, mid_thr)};

  // 2^32 * (
  //      2^32 * (this_h * rhs_h)
  //        + (this_h + this_l) * (rhs_h + rhs_l)
  //        - this_h * rhs_h
  //        - this_l * rhs_l
  //  ) + this_l * rhs_l
  SmallInt mix = (this_l + this_h) * (rhs_l + rhs_h);
  SmallInt ups = this_h * rhs_h;
  SmallInt lws = this_l * rhs_l;

  UResetBinary(ups.ToView());
  *this <<= mid_thr * kWordBSize;
  UAddRange(mix.ToView());
  USubRange(ups.ToView());
  USubRange(lws.ToView());
  *this <<= mid_thr * kWordBSize;
  UAddRange(lws.ToView());
}

template<std::size_t cap, typename W, typename DW>
constexpr void BigInt<cap, W, DW>::UMulByRange(
    const RandomAccessRange<W> auto& range) noexcept {
  std::size_t range_wc = std::ranges::size(range);
  if (words_count == 1 || range_wc == 1) {
    UMulByShortRange(range);
  } else if constexpr (cap < 40) {
    BigInt ret;
    auto it = std::ranges::begin(range);
    for (std::size_t i = 0; i < range_wc; ++i, ++it) {
      if (*it == 0) {
        continue;
      }
      BigInt tmp{std::ranges::single_view(*it)};
      tmp.UMulByShortRange(ToView());
      tmp <<= i * kWordBSize;
      ret.UAddRange(tmp.ToView());
    }
    UResetBinary(ret.ToView());
  } else {
    std::size_t max_size = words_count + range_wc;
#define TRY_OPTIMIZE(small_cap)                                                \
  if (max_size <= small_cap) {                                                 \
    KaratsubaUMulByRange<small_cap>(range);                                    \
    return;                                                                    \
  }

    TRY_OPTIMIZE(cap / 16 + 1);
    TRY_OPTIMIZE(cap / 8 + 1);
    TRY_OPTIMIZE(cap / 4 + 1);
    TRY_OPTIMIZE(cap / 2 + 1);
#undef TRY_OPTIMIZE
    KaratsubaUMulByRange<cap>(range);
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator*=(const BigInt& rhs) noexcept {
  is_positive ^= !rhs.is_positive;

  if (rhs.IsPowerOf2()) {
    *this <<= rhs.BitWidth() - 1;
  } else {
    UMulByRange(rhs.ToView());
  }

  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr W BigInt<cap, W, DW>::UDivByWord(W rhs) noexcept {
  BigInt remainder;
  DW window = 0;
  for (std::size_t i = 0; i < words_count; ++i) {
    window = (window << kWordBSize) + binary[words_count - 1 - i];
    if (window >= rhs) {
      binary[words_count - 1 - i] = window / rhs;
      window %= rhs;
    } else {
      binary[words_count - 1 - i] = 0;
    }
  }

  if (words_count > 1 && binary[words_count - 1] == 0) {
    words_count -= 1;
  }

  return static_cast<W>(window);
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> BigInt<cap, W, DW>::UDivBySameRange(
    const RandomAccessRange<W> auto& range) noexcept {
  if (IsZero()) {
    UResetBinary(std::ranges::single_view(W{0}));
    return 0;
  }

  ASSERT(BitWidth() - RangeBitWidth(range) <= kWordBSize,
         "Same division not applicable");

  // Use binary search for division
  W l{0}; // unreachable edge
  W r{kMaxWord};
  while (l < r) {
    // ceil((l + r) / 2) without overflow
    W m = (l >> 1) + (r >> 1) + ((l | r) & 1);

    BigInt q{ToView()};
    q.UDivByWord(m);
    if (auto cmp = q.UCompare(range); cmp < 0) {
      r = m - 1;
    } else {
      l = m;
    }
  }

  BigInt remainder{r};
  remainder.UMulByRange(range);
  remainder.USubRange(ToView());

  UResetBinary(std::ranges::single_view(r));
  return remainder;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> BigInt<cap, W, DW>::UDivByRange(
    const RandomAccessRange<W> auto& range) noexcept {
  if (auto cmp = UCompare(range); cmp < 0) {
    BigInt remainder{ToView()};
    UResetBinary(std::ranges::single_view(W{0}));
    return remainder;
  } else if (cmp == 0) {
    UResetBinary(std::ranges::single_view(W{1}));
    return BigInt{};
  } else if (std::ranges::size(range) == 1) {
    return UDivByWord(*std::ranges::begin(range));
  } else if (BitWidth() - RangeBitWidth(range) < kWordBSize) {
    return UDivBySameRange(range);
  } else {
    BigInt r, q; // remainder and quotient
    for (std::size_t i = 0; i < words_count; ++i) {
      q <<= kWordBSize;
      r <<= kWordBSize;
      r.UAddRange(std::ranges::single_view(binary[words_count - 1 - i]));
      if (auto cmp = r.UCompare(range); cmp >= 0) {
        BigInt tmp = r;
        r.UResetBinary(tmp.UDivBySameRange(range).ToView());
        ASSERT(tmp.words_count == 1);
        q.UAddRange(tmp.ToView());
      }
    }
    UResetBinary(q.ToView());
    return r;
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator/=(const BigInt& rhs) noexcept {
  ASSERT(!rhs.IsZero(), "Division by zero");
  is_positive ^= !rhs.is_positive;
  if (rhs.IsPowerOf2()) {
    *this >>= rhs.BitWidth() - 1;
  } else {
    UDivByRange(rhs.ToView());
  }
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator%=(const BigInt& rhs) noexcept {
  ASSERT(!rhs.IsZero(), "Division by zero");
  is_positive = rhs.is_positive;
  UResetBinary(UDivByRange(rhs.ToView()).ToView());
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> BigInt<cap, W, DW>::operator~() const noexcept {
  static_assert(!kInfInt, "Can't negate unbound BigInt");
  BigInt ret = *this;
  if (ret.words_count < cap) {
    for (std::size_t i = 0; i < ret.words_count; ++i) {
      ret.binary[i] = ~ret.binary[i];
    }
    for (std::size_t i = ret.words_count; i < cap; ++i) {
      ret.binary[i] = ~0;
    }
    ret.words_count = cap;
  } else {
    ret.words_count = 1;
    for (std::size_t i = 0; i < cap; ++i) {
      std::size_t idx = cap - 1;
      if ((ret.binary[idx] = ~ret.binary[idx]) && ret.words_count == 1) {
        ret.words_count = idx + 1;
      }
    }
  }

  return ret;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator^=(const BigInt& other) noexcept {
  if (words_count != other.words_count) {
    std::size_t iters = std::min(words_count, other.words_count);
    for (std::size_t i = 0; i < iters; ++i) {
      binary[i] ^= other.binary[i];
    }

    if (words_count < other.words_count) {
      for (std::size_t i = iters; i < other.words_count; ++i) {
        binary[i] = other.binary[i];
      }
      words_count = other.words_count;
    }
  } else {
    for (std::size_t i = 0; i < words_count; ++i) {
      binary[i] ^= other.binary[i];
    }
  }
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator|=(const BigInt& other) noexcept {
  std::size_t iters = std::min(words_count, other.words_count);
  for (std::size_t i = 0; i < iters; ++i) {
    binary[i] |= other.binary[i];
  }

  if (iters < other.words_count) {
    for (std::size_t i = iters; i < other.words_count; ++i) {
      binary[i] = other.binary[i];
    }
    words_count = other.words_count;
  }

  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW>&
BigInt<cap, W, DW>::operator&=(const BigInt& other) noexcept {
  const std::size_t iterations = std::min(words_count, other.words_count);
  words_count = 1;
  for (std::size_t i = 0; i < iterations; ++i) {
    if ((binary[i] &= other.binary[i])) {
      words_count = i + 1;
    }
  }
  return *this;
}

template<std::size_t cap, typename W, typename DW>
constexpr bool BigInt<cap, W, DW>::RangeIsZero(
    const RandomAccessRange<W> auto& range) noexcept {
  return std::ranges::size(range) == 1 && *std::ranges::begin(range) == 0;
}

template<std::size_t cap, typename W, typename DW>
constexpr bool BigInt<cap, W, DW>::IsZero() const noexcept {
  return RangeIsZero(ToView());
}

template<std::size_t cap, typename W, typename DW>
constexpr bool BigInt<cap, W, DW>::IsPowerOf2() const noexcept {
  if (IsZero()) {
    return false;
  }
  for (std::size_t i = 0; i < words_count - 1; ++i) {
    if (binary[i]) {
      return false;
    }
  }
  return (binary[words_count - 1] & (binary[words_count - 1] - 1)) == 0;
}

template<std::size_t cap, typename W, typename DW>
constexpr std::string BigInt<cap, W, DW>::ToString(W base) const noexcept {
  constexpr std::string_view alphabet = "0123456789"
                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ASSERT(base >= 2 && base <= 36);
  if (IsZero()) {
    return "0";
  }

  std::string ret;
  BigInt copy{ToView()};

  while (!copy.IsZero()) {
    ret.push_back(alphabet.at(copy.UDivByWord(base)));
  }

  if (!is_positive) {
    ret.push_back('-');
  }
  std::reverse(ret.begin(), ret.end());
  return ret;
}

template<std::size_t cap, typename W, typename DW>
constexpr uint64_t BigInt<cap, W, DW>::ToUint() const noexcept {
  ASSERT(words_count * kWordBSize <= 64);
  uint64_t ret = 0;
  for (W w : std::ranges::reverse_view(ToView())) {
    ret = (ret << kWordBSize) + w;
  }
  return ret;
}

template<std::size_t cap, typename W, typename DW>
constexpr std::size_t BigInt<cap, W, DW>::RangeBitWidth(
    const RandomAccessRange<W> auto& range) noexcept {
  std::size_t range_words_count = std::ranges::size(range);
  return (range_words_count - 1) * kWordBSize +
         std::bit_width(std::ranges::begin(range)[range_words_count - 1]);
}

template<std::size_t cap, typename W, typename DW>
constexpr std::size_t BigInt<cap, W, DW>::BitWidth() const noexcept {
  return RangeBitWidth(ToView());
}

template<std::size_t cap, typename W, typename DW>
constexpr auto BigInt<cap, W, DW>::ToView() const noexcept {
  return std::views::counted(binary.cbegin(), words_count);
}

template<std::size_t cap, typename W, typename DW>
constexpr std::strong_ordering BigInt<cap, W, DW>::UCompare(
    const RandomAccessRange<W> auto& range) const noexcept {
  if (auto cmp = BitWidth() <=> RangeBitWidth(range); cmp != 0) {
    return cmp;
  }

  auto range_data = std::ranges::begin(range);
  for (std::size_t i = 0; i < words_count; ++i) {
    if (auto cmp =
            binary[words_count - 1 - i] <=> range_data[words_count - 1 - i];
        cmp != 0) {
      return cmp;
    }
  }
  return std::strong_ordering::equal;
}

template<std::size_t cap, typename W, typename DW>
constexpr bool
BigInt<cap, W, DW>::operator==(const BigInt& rhs) const noexcept {
  return (*this <=> rhs) == 0;
}

template<std::size_t cap, typename W, typename DW>
constexpr std::strong_ordering
BigInt<cap, W, DW>::operator<=>(const BigInt& rhs) const noexcept {
  if (IsZero() && rhs.IsZero()) {
    return std::strong_ordering::equal;
  } else if (is_positive ^ rhs.is_positive) {
    return is_positive <=> rhs.is_positive;
  } else if (auto cmp = UCompare(rhs.ToView()); is_positive) {
    return cmp;
  } else {
    return 0 <=> cmp;
  }
}

// Arithmetic opeartors
template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator+(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs += rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator+(const BigInt<cap, W, DW>& lhs,
                                       T&& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(rhs)} + lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator+(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} + rhs;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator-(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs -= rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator-(const BigInt<cap, W, DW>& lhs,
                                       T&& rhs) noexcept {
  return -(BigInt<cap, W, DW>{std::forward<T>(rhs)} - lhs);
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator-(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} - rhs;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator*(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs *= rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator*(const BigInt<cap, W, DW>& lhs,
                                       T&& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(rhs)} * lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator*(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} * rhs;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator/(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs /= rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator/(BigInt<cap, W, DW> lhs,
                                       T&& rhs) noexcept {
  lhs /= BigInt<cap, W, DW>{std::forward<T>(rhs)};
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator/(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} / rhs;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator%(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs %= rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator%(BigInt<cap, W, DW> lhs,
                                       T&& rhs) noexcept {
  lhs %= BigInt<cap, W, DW>{std::forward<T>(rhs)};
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator%(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} % rhs;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator&(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs &= rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator&(const BigInt<cap, W, DW>& lhs,
                                       T&& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(rhs)} & lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator&(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} & rhs;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator|(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs |= rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator|(const BigInt<cap, W, DW>& lhs,
                                       T&& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(rhs)} | lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator|(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} | rhs;
}

template<std::size_t cap, typename W, typename DW>
constexpr BigInt<cap, W, DW> operator^(BigInt<cap, W, DW> lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  lhs ^= rhs;
  return lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator^(const BigInt<cap, W, DW>& lhs,
                                       T&& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(rhs)} ^ lhs;
}

template<std::size_t cap, typename W, typename DW, typename T>
  requires(!IsBigInt<std::remove_cvref_t<T>>::value)
constexpr BigInt<cap, W, DW> operator^(T&& lhs,
                                       const BigInt<cap, W, DW>& rhs) noexcept {
  return BigInt<cap, W, DW>{std::forward<T>(lhs)} ^ rhs;
}

} // namespace algo
