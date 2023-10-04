#include "utils.hpp"
#include <algo/bigint.hpp>

#include <gtest/gtest.h>

#include <bitset>
#include <fstream>

template<typename T>
struct Converter {};

template<std::size_t capacity, typename Word, typename _>
struct Converter<algo::BigInt<capacity, Word, _>> {

  template<typename T>
  algo::BigInt<capacity, Word, _> operator()(T val) {
    static_assert(!std::numeric_limits<T>::is_signed);
    static_assert(std::numeric_limits<T>::digits >=
                  std::numeric_limits<Word>::digits * capacity);

    auto masked = [](T val, std::size_t part) -> Word {
      std::size_t shift = part * 8;
      T mask = std::numeric_limits<Word>::max();
      return static_cast<Word>((val & (mask << shift)) >> shift);
    };

    auto build = [&masked]<std::size_t... Indicies>(
                     T val,
                     std::integer_sequence<std::size_t, Indicies...> seq) {
      Word arr[] = {masked(val, Indicies)...};
      return algo::BigInt<capacity, Word, _>{arr};
    };

    return build(val, std::make_integer_sequence<std::size_t, capacity>{});
  }
};

struct BigInt : algo::testing::Randomizer {

  std::string RandomBinary(std::size_t size) {
    return "0b1" + RandomString(size - 1, "01");
  }

  static std::string NaiveAdd(std::string_view lhs, std::string_view rhs) {
    if (lhs.starts_with("0b")) {
      lhs.remove_prefix(2);
    }
    if (rhs.starts_with("0b")) {
      rhs.remove_prefix(2);
    }

    bool overflow = false;
    std::string ret;
    ret.reserve(std::max(lhs.size(), rhs.size()) + 1);

    auto lhs_it{lhs.rbegin()}, rhs_it{rhs.rbegin()};

    while (lhs_it != lhs.rend() || rhs_it != rhs.rend()) {
      bool lhs_bit = lhs_it != lhs.rend() ? *lhs_it == '1' : false;
      bool rhs_bit = rhs_it != rhs.rend() ? *rhs_it == '1' : false;

      if (rhs_bit && overflow) {
        ret.push_back(lhs_bit ? '1' : '0');
      } else if (lhs_bit && (rhs_bit || overflow)) {
        ret.push_back('0');
        overflow = true;
      } else {
        ret.push_back(lhs_bit || rhs_bit || overflow ? '1' : '0');
        overflow = false;
      }

      if (lhs_it != lhs.rend()) {
        ++lhs_it;
      }
      if (rhs_it != rhs.rend()) {
        ++rhs_it;
      }
    }

    if (overflow) {
      ret.push_back('1');
    }
    ret.push_back('b');
    ret.push_back('0');

    std::reverse(ret.begin(), ret.end());
    return ret;
  }

  static std::string NaiveMul(std::string_view lhs, std::string_view rhs) {
    std::string ret{"0b0"};
    for (std::size_t j = 0; j < rhs.size(); ++j) {
      if (rhs[rhs.size() - j - 1] == '1') {
        std::string tmp{lhs};
        tmp.resize(tmp.size() + j, '0');
        ret = NaiveAdd(ret, tmp);
      }
    }
    return ret;
  }

  static std::string Words(const auto& bi) {
    std::stringstream ss;
    ss << bi.words_count << ' ' << (bi.is_positive ? '+' : '-');

    for (auto v : std::ranges::reverse_view(bi.ToView())) {
      static_assert(std::numeric_limits<decltype(v)>::digits <= 32);
      ss << ' ' << static_cast<uint32_t>(v);
    }

    return std::move(ss).str();
  }

  template<typename T, typename... Operands>
  static std::string Format(bool as_words, const T& actual, const T& expected,
                            Operands&&... ops) {
    std::string delim = "\n----------------------------\n";
    std::stringstream ss;
    ss << delim;
    if (as_words) {
      ss << "Actual: " << actual << delim;
      ss << "Expected: " << expected << delim;
    } else {
      ss << "Actual: " << actual.ToString() << delim;
      ss << "Expected: " << expected.ToString() << delim;
    }

    auto print_operands = [&]<std::size_t... Indicies>(
                              std::integer_sequence<std::size_t, Indicies...>) {
      if (as_words) {
        ((ss << "Operand " << Indicies + 1 << ": " << ops << delim), ...);
      } else {
        ((ss << "Operand " << Indicies + 1 << ": " << ops.ToString() << delim),
         ...);
      }
    };

    print_operands(std::make_integer_sequence<std::size_t, sizeof...(ops)>{});
    return std::move(ss).str();
  }
};

TEST_F(BigInt, Shift) {
  using Int = algo::BigInt<3, uint8_t, uint16_t>;
  Converter<Int> convert;

  uint32_t cap = std::numeric_limits<uint16_t>::max();
  for (uint32_t n = 0; n < cap; ++n) {
    for (std::size_t shift = 0; shift < 24; ++shift) {
      ASSERT_EQ(convert(n) << shift, convert(n << shift)) << n << '\t' << shift;

      uint32_t nn = n << 8;
      ASSERT_EQ(convert(nn) >> shift, convert(nn >> shift))
          << n << '\t' << shift;
    }
  }
}

TEST_F(BigInt, Add) {
  {
    using Int = algo::BigInt<2, uint8_t, uint16_t>;
    uint16_t max = 0b1111'11111111;
    for (uint16_t i = 0; i < max; ++i) {
      for (uint16_t j = 0; j < max; j += 10) {
        ASSERT_EQ(Int{i} + Int{j}, Int{static_cast<uint16_t>(i + j)});
      }
    }
  }

  {
    using Int = algo::BigInt<30, uint8_t, uint16_t>;
    Int lhs{"278905768535045230537762672916647633790"};
    Int rhs{"86877037227277156296489520973326592245760"};
    ASSERT_EQ(lhs + rhs, Int{"87155942995812201527027283646243239879550"});
  }
}

TEST_F(BigInt, Sub) {
  {
    using Int = algo::BigInt<4, uint8_t, uint16_t>;
    Int lhs{"21310592"};
    Int rhs{"21299110"};

    ASSERT_EQ(lhs - rhs, Int{"11482"});
  }

  {
    using Int = algo::BigInt<2, uint8_t, uint16_t>;
    uint16_t max = 0b1111'11111111;
    for (uint16_t i = 0; i < max; ++i) {
      for (uint16_t j = 0; j < i; j += 10) {
        ASSERT_EQ(Int{i} - Int{j}, Int{static_cast<uint16_t>(i - j)});

        ASSERT_EQ(Int{j} - Int{i}, Int(static_cast<uint16_t>(i - j), false));
      }
    }
  }
}

TEST_F(BigInt, MulShort) {
  using Int = algo::BigInt<4, uint8_t, uint16_t>;
  Converter<Int> convert;

  for (uint16_t i = 0b1'00000000; i <= 0b1111'11111111; ++i) {
    for (uint16_t j = 0; j < std::numeric_limits<uint8_t>::max(); ++j) {
      uint32_t mul = i;
      mul *= j;

      ASSERT_EQ(Int{i} * Int{j}, convert(mul));
      ASSERT_EQ(Int{j} * Int{i}, convert(mul));
    }
  }
}

TEST_F(BigInt, Mul) {
  {
    using Int = algo::BigInt<4, uint8_t, uint16_t>;
    Converter<Int> convert;

    for (uint16_t i = 0b1'00000000; i <= 0b1111'11111111; ++i) {
      for (uint16_t j = 0b1'00000000; j <= 0b1111'11111111; j += 15) {
        uint32_t mul = i;
        mul *= j;

        Int lhs{i};
        Int rhs{j};
        ASSERT_EQ(lhs * rhs, convert(mul));
        ASSERT_EQ(lhs * rhs, rhs * lhs);
      }
    }
  }

  {
    using Int = algo::BigInt<10, uint8_t, uint16_t>;

    Int lhs{"67391"};
    Int rhs{"11482"};
    ASSERT_EQ(lhs * rhs, Int{"773783462"});
  }

  {
    using Int = algo::BigInt<20, uint8_t, uint16_t>;

    Int lhs{"1130648259085"};
    Int rhs{"192638812232"};
    ASSERT_EQ(lhs * rhs, Int{"217806737682313003127720"});
  }
}

TEST_F(BigInt, MulKaratsuba) {
  constexpr std::size_t cap = 100;
  using Int = algo::BigInt<cap, uint8_t, uint16_t>;

  SetSeed(1);
  for (std::size_t i = 0; i < 100; ++i) {
    std::string lhs_str =
        RandomBinary(RandomInt<std::size_t>(cap * 3.2, cap * 4));

    std::string rhs_str =
        RandomBinary(RandomInt<std::size_t>(cap * 3.2, cap * 4));

    std::string mul_str = NaiveMul(lhs_str, rhs_str);

    {
      Int lhs{lhs_str};
      Int rhs{rhs_str};
      ASSERT_EQ(lhs * rhs, Int{mul_str}) << lhs << '\n' << rhs;
    }
  }
}

TEST_F(BigInt, Serialize) {
  using Int = algo::BigInt<8, uint8_t, uint16_t>;
  SetSeed(1);
  for (std::size_t i = 0; i < 100; ++i) {
    std::string str = RandomBinary(RandomInt<std::size_t>(50, 64));
    ASSERT_EQ(Int{str}.ToString(2), str.substr(2));
  }

  for (std::size_t i = 0; i < 100; ++i) {
    std::string str =
        RandomString(1, "123456789") +
        RandomString(RandomInt<std::size_t>(14, 17), "0123456789");
    ASSERT_EQ(Int{str}.ToString(), str);
  }
}

TEST_F(BigInt, DivShort) {
  using Int = algo::BigInt<8, uint8_t, uint16_t>;

  {
    for (uint16_t i = 0; i < 0b1111'11111111; ++i) {
      for (uint16_t j = 1; j < 255; ++j) {
        ASSERT_EQ(Int{i} / Int{j}, Int{static_cast<uint16_t>(i / j)});
        ASSERT_EQ(Int{i} % Int{j}, Int{static_cast<uint16_t>(i % j)});
      }
    }
  }

  {
    Converter<Int> convert;

    for (uint64_t i = 123456789123; i < 12345678912378234ull;
         i += 9308274565421) {
      Int div{convert(i)};
      for (uint64_t j = 1; j < std::numeric_limits<uint8_t>::max(); ++j) {
        ASSERT_EQ(div / convert(j), convert(i / j));
        ASSERT_EQ(div % convert(j), convert(i % j));
      }
    }
  }
}

TEST_F(BigInt, Div) {
  using Int = algo::BigInt<8, uint8_t, uint16_t>;

  {
    ASSERT_EQ(Int{"123124123"} / Int{"123124123"}, Int{1});
    ASSERT_EQ(Int{"123124123"} % Int{"123124123"}, Int{0});
  }

  {
    Converter<Int> convert;

    SetSeed(1);
    for (std::size_t i = 0; i < 1'000; ++i) {
      uint64_t divident = RandomInt<uint64_t>();
      uint64_t divisor = RandomInt<uint64_t>(1, divident);

      ASSERT_EQ(convert(divident) / convert(divisor),
                convert(divident / divisor))
          << divident << '\n'
          << divisor;

      ASSERT_EQ(convert(divident) % convert(divisor),
                convert(divident % divisor));
    }

    for (std::size_t i = 0; i < 1'000; ++i) {
      std::size_t divident_len = RandomInt<std::size_t>(1, 64);
      std::size_t divisor_len = RandomInt<std::size_t>(1, divident_len);

      Int divident{RandomBinary(divident_len)};
      Int divisor{RandomBinary(divisor_len)};

      ASSERT_EQ(divident / divisor,
                convert(divident.ToUint() / divisor.ToUint()));

      ASSERT_EQ(divident % divisor,
                convert(divident.ToUint() % divisor.ToUint()));
    }
  }
}

template<typename T>
struct PowerModulo;

template<std::size_t cap, typename W, typename DW>
struct PowerModulo<algo::BigInt<cap, W, DW>> {
  using Int = algo::BigInt<cap, W, DW>;
  using DInt = algo::BigInt<cap * 2, W, DW>;

  PowerModulo(Int mod) noexcept
      : modulo{std::move(mod)}
      , big_modulo{modulo.ToView()} {
  }

  void operator()(const Int& base, Int&& exp, Int& res) {
    if (exp.IsZero()) {
      res = one;
    } else if (exp == one) {
      res = base;
    } else {
      bool exp_even = (exp & one).IsZero();
      (*this)(base, std::move(exp >>= 1), res);
      MulSelf(res, res);
      if (!exp_even) {
        MulSelf(res, base);
      }
    }
  }

  void MulSelf(Int& self, const Int& rhs) {
    if (self.words_count + rhs.words_count > cap) {
      DInt bigger{self.ToView()};
      bigger *= {rhs.ToView()};
      bigger %= big_modulo;
      self = bigger.ToView();
    } else {
      self *= rhs;
      self %= modulo;
    }
  }

  Int one{1};
  Int modulo;
  DInt big_modulo;
};

TEST_F(BigInt, Algebra) {
  using Int = algo::BigInt<32>;
  // https://oeis.org/A000043
  Int big_prime = (Int{1} << 127) - Int{1};
  PowerModulo<Int> pow{big_prime};

  for (uint64_t i = 1; i < 100; ++i) {
    uint64_t n = RandomInt<uint64_t>(2, std::numeric_limits<uint64_t>::max());

    Int ret;
    pow(Int{n}, big_prime - Int{1}, ret);
    ASSERT_EQ(ret, Int{1}) << i << '\t' << n;
  }
}
