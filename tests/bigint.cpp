#include "utils.hpp"
#include <algo/bigint.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <bitset>

template<typename T>
struct Converter {};

template<size_t capacity, typename Word, typename _>
struct Converter<algo::BigInt<capacity, Word, _>> {

    template<typename T>
    algo::BigInt<capacity, Word, _> operator()(T val) {
        static_assert(!std::numeric_limits<T>::is_signed);
        static_assert(std::numeric_limits<T>::digits
                       >= std::numeric_limits<Word>::digits * capacity);

        auto masked = [](T val, size_t part) -> Word {
            size_t shift = part * 8;
            T mask = std::numeric_limits<Word>::max();
            return static_cast<Word>((val & (mask << shift)) >> shift);
        };

        auto build = [&masked]<size_t... Indicies>(
                T val,
                std::integer_sequence<size_t, Indicies...> seq)
        {
            Word arr[] = {masked(val, Indicies)...};
            return algo::BigInt<capacity, Word, _>{arr};
        };

        return build(val, std::make_integer_sequence<size_t, capacity>{});
    }

};

struct BigInt : algo::testing::Randomizer {

    std::string RandomBinary(size_t size) {
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

        auto lhs_it {lhs.rbegin()}, rhs_it {rhs.rbegin()};

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
        std::string ret {"0b0"};
        for (size_t j = 0; j < rhs.size(); ++j) {
            if (rhs[rhs.size() - j - 1] == '1') {
                std::string tmp {lhs};
                tmp.resize(tmp.size() + j, '0');
                ret = NaiveAdd(ret, tmp);
            }
        }
        return ret;
    }

    template<typename T, typename... Operands>
    static std::string Format(bool as_words,
                              const T& actual,
                              const T& expected,
                              Operands&&... ops)
    {
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

        auto print_operands = [&]<size_t... Indicies>(
                std::integer_sequence<size_t, Indicies...>)
        {
            if (as_words) {
                ((ss << "Operand " << Indicies + 1 << ": "
                     << ops << delim), ...);
            } else {
                ((ss << "Operand " << Indicies + 1 << ": "
                     << ops.ToString() << delim), ...);
            }
        };

        print_operands(std::make_integer_sequence<size_t, sizeof...(ops)>{});
        return std::move(ss).str();
    }

};

TEST_F(BigInt, Shift) {
    using Int = algo::BigInt<3, uint8_t, uint16_t>;
    Converter<Int> convert;

    uint32_t cap = std::numeric_limits<uint16_t>::max();
    for (uint32_t n = 0; n < cap; ++n) {
        for (size_t shift = 0; shift < 24; ++shift) {
            ASSERT_EQ(convert(n) << shift, convert(n << shift))
                    << n << '\t' << shift;

            uint32_t nn = n << 8;
            ASSERT_EQ(convert(nn) >> shift, convert(nn >> shift))
                    << n << '\t' << shift;
        }
    }
}

TEST_F(BigInt, Add) {
    using Int = algo::BigInt<2, uint8_t, uint16_t>;
    uint16_t max = 0b1111'11111111;
    for (uint16_t i = 0; i < max; ++i) {
        for (uint16_t j = 0; j < max; j += 10) {
            ASSERT_EQ(Int{i} + Int{j}, Int{static_cast<uint16_t>(i + j)});
        }
    }
}

TEST_F(BigInt, Sub) {
    using Int = algo::BigInt<2, uint8_t, uint16_t>;
    uint16_t max = 0b1111'11111111;
    for (uint16_t i = 0; i < max; ++i) {
        for (uint16_t j = 0; j < i; j += 10) {
            ASSERT_EQ(Int{i} - Int{j},
                      Int{static_cast<uint16_t>(i - j)});

            ASSERT_EQ(Int{j} - Int{i},
                      Int(static_cast<uint16_t>(i - j), false));
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
    using Int = algo::BigInt<4, uint8_t, uint16_t>;
    Converter<Int> convert;

    for (uint16_t i = 0b1'00000000; i <= 0b1111'11111111; ++i) {
        for (uint16_t j = 0b1'00000000; j <= 0b1111'11111111; j += 10) {
            uint32_t mul = i;
            mul *= j;

            Int lhs {i};
            Int rhs {j};
            ASSERT_EQ(lhs * rhs, convert(mul));
            ASSERT_EQ(lhs * rhs, rhs * lhs);
        }
    }
}

TEST_F(BigInt, MulKaratsuba) {
    using Int = algo::BigInt<100, uint8_t, uint16_t>;
    SetSeed(1);

    for (size_t i = 0; i < 100; ++i) {
        std::string lhs_str = RandomBinary(RandomInt<size_t>(320, 400));
        std::string rhs_str = RandomBinary(RandomInt<size_t>(320, 400));
        std::string mul_str = NaiveMul(lhs_str, rhs_str);

        {
            Int lhs {lhs_str};
            Int rhs {rhs_str};
            ASSERT_EQ(lhs * rhs, Int{mul_str})
                << lhs.ToString() << '\n' << rhs.ToString();
        }
    }
}

TEST_F(BigInt, Serialize) {
    using Int = algo::BigInt<8, uint8_t, uint16_t>;
    SetSeed(1);
    for (size_t i = 0; i < 100; ++i) {
        std::string str = RandomBinary(RandomInt<size_t>(50, 64));
        ASSERT_EQ(Int{str}.ToString(2), str.substr(2));
    }

    for (size_t i = 0; i < 100; ++i) {
        std::string str = RandomString(1, "123456789")
                        + RandomString(RandomInt<size_t>(14, 17), "0123456789");
        ASSERT_EQ(Int{str}.ToString(), str);
    }
}

TEST_F(BigInt, Tmp) {
    using Int = algo::BigInt<100, uint8_t, uint16_t>;
    std::string lhs = "62902805278564175885450201171482551592362697531405419301362439180089461679894169277912768041299508395833040290";
    std::string rhs = "1124654617929823523508418266605769438880489315693959666619759738912975269702691131329786004609322592816605481539";
    std::string mul = "70743930437277679583050105498855453208622151128881564088493104886725311177418596746691228539997892898307455561897755738878238501464820919615400067064898595858377940593250734611037776627002322512144977288531782020124480870";
    std::string true_mul = "70743930437277679583050105498855453208622151128881564088493104886725311177418596746691228539997892420591228115819666652533191680907624168619828393658548722186396980573440661495416770142611965686646161646444967061838206310";

    // ASSERT_EQ(Int{lhs} * Int{rhs}, Int{mul});
    Int llhs {lhs};
    Int rrhs {rhs};
    ASSERT_EQ(llhs * rrhs, rrhs * llhs);
    ASSERT_EQ(llhs * rrhs, Int{true_mul}) << llhs << '\n' << rrhs;
}

//TEST_F(BigInt, Tmp) {
    //using namespace algo::literals;
    //using Int = algo::BigInt<100, uint8_t, uint16_t>;

    //std::fstream f {"test_data.txt"};
    //std::string lhs_str, rhs_str;

    //f >> lhs_str;
    //f >> rhs_str;

    //std::string mul = "70743930437277679583050105498855453208622151128881564088493104886725311177418596746691228539997892813093209585029772280232905825365429066735108903700522672446619174751879208055278353848705664267596539903727106919457199462";

    //{
        //Int lhs {lhs_str};
        //Int rhs {rhs_str};
        //ASSERT_EQ(lhs * rhs, Int{mul});
    //}
//}


//TEST_F(TestBigInt, Mul) {
    //{
        //BI32 bi = (1ull << 33) - 1;
        //ASSERT_EQ(bi * 3ull, ((1ull << 33) - 1) * 3ull);
    //}

    //SetSeed(2);
    //for (size_t i = 0; i < 100; ++i) {
        //std::string lhs = RandomBinary(RandomInt(48, 53));
        //std::string rhs = RandomBinary(RandomInt(8, 10));
        //std::string mul = NaiveMul(lhs, rhs);

        //BI32 lhs_bi {lhs};
        //BI32 rhs_bi {rhs};
        //BI32 mul_bi {mul};

        //ASSERT_EQ(lhs_bi * rhs_bi, mul_bi);
        //ASSERT_EQ(mul_bi / lhs_bi, rhs_bi);
        //ASSERT_EQ(mul_bi / rhs_bi, lhs_bi);
    //}

    //for (size_t i = 0; i < 100; ++i) {
        //std::string lhs = RandomBinary(RandomInt(20, 512));
        //std::string rhs = RandomBinary(RandomInt(28, 512));
        //std::string mul = NaiveMul(lhs, rhs);

        //BI32 lhs_bi {lhs};
        //BI32 rhs_bi {rhs};
        //BI32 mul_bi {mul};

        //ASSERT_EQ(lhs_bi * rhs_bi, mul_bi);
        //ASSERT_EQ(rhs_bi * lhs_bi, mul_bi);
        //ASSERT_EQ(mul_bi / lhs_bi, rhs_bi);
        //ASSERT_EQ(mul_bi / rhs_bi, lhs_bi);
    //}

    //for (size_t i = 0; i < 100; ++i) {
        //std::string lhs = RandomBinary(RandomInt(500, 512));
        //std::string rhs = RandomBinary(RandomInt(500, 512));
        //std::string mul = NaiveMul(lhs, rhs);

        //BI32 lhs_bi {lhs};
        //BI32 rhs_bi {rhs};
        //BI32 mul_bi {mul};

        //ASSERT_EQ(lhs_bi * rhs_bi, mul_bi);
        //ASSERT_EQ(rhs_bi * lhs_bi, mul_bi);
        //ASSERT_EQ(mul_bi / lhs_bi, rhs_bi);
        //ASSERT_EQ(mul_bi / rhs_bi, lhs_bi);
        //ASSERT_EQ(lhs_bi / mul_bi, 0);
        //ASSERT_EQ(mul_bi % lhs_bi, 0);
    //}

    //for (size_t i = 0; i < 100; ++i) {
        //std::string q = RandomBinary(RandomInt(500, 500));
        //std::string d = RandomBinary(RandomInt(400, 400));
        //std::string r = RandomBinary(RandomInt(100, 399));
        //std::string m = NaiveAdd(NaiveMul(q, d), r);

        //BI32 m_bi {m};
        //BI32 d_bi {d};

        //ASSERT_EQ(m_bi / d_bi, BI32{q});
        //ASSERT_EQ(m_bi % d_bi, BI32{r});
    //}
//}

//TEST_F(BigInt, BigInt) {
    //{
        //BI32 a {"0b11101100"};
        //ASSERT_EQ(a, 0b11101100);
        //BI32 b {"0b1001" "11110000"
                         //"10100000"
                         //"11110000"
                         //"10100000"};
        //ASSERT_EQ(b, 0b1001'1111'0000'1010'0000'1111'0000'1010'0000ull);
    //}

    //{
        //ASSERT_EQ(BI32{"0b110111101011110011110101110111001001001"}, BI32{478326484553ull});
    //}

    //{
        //BI32 a {12345};
        //ASSERT_EQ(a.ToString(10), "12345");
        //ASSERT_EQ(a.ToString(36), "9IX");
        //ASSERT_EQ(BI32{478326484553ull}.ToString(10), "478326484553");

        //ASSERT_EQ(BI32{"729850759413822409864896864703110513397153737649725935393728469480268371494607235845025611298670241485102205454354037818905499"}.ToString(),
                       //"729850759413822409864896864703110513397153737649725935393728469480268371494607235845025611298670241485102205454354037818905499");
    //}
//}

//void PowerModulo(const BI32& base, BI32& res, BI32&& exp, const BI32& mod) {
    //if (exp.IsZero()) {
        //res = 1;
    //} else if (exp == 1) {
        //if (base >= mod) {
            //res = base % mod;
        //} else {
            //res = base;
        //}
    //} else {
        //bool is_odd = exp % 2ull == 1ull;
        //PowerModulo(base, res, std::move(exp >>= 1), mod);
        //if (res.words_count * 2 > 32) {
            //algo::BigInt<64> tmp {res.ToView()};
            //tmp *= tmp;
            //tmp %= algo::BigInt<64>{mod.ToView()};
            //res = BI32{mod.ToView()};
        //} else {
            //res *= res;
            //res %= mod;
        //}

        //if (is_odd) {
            //if (base.words_count + res.words_count > 32) {
                //algo::BigInt<64> tmp {res.ToView()};
                //tmp *= algo::BigInt<64>{base.ToView()};
                //tmp %= algo::BigInt<64>{mod.ToView()};
                //res = BI32{mod.ToView()};
            //} else {
                //res *= base;
                //res %= mod;
            //}
        //}
    //}
//}

//TEST_F(TestBigInt, Algebra) {
    //// https://oeis.org/A000043
    //BI32 big_prime {1};
    //big_prime <<= 606;
    //big_prime -= 1;

    //for (size_t i = 2; i < 1000; ++i) {
        //BI32 base {i};
        //BI32 res;
        //PowerModulo(base, res, big_prime - 1ull, big_prime);
        //ASSERT_EQ(res, 1);
    //}
//}
