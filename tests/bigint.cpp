#include "utils.hpp"
#include <algo/bigint.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <string_view>
#include <bitset>

using BI32 = algo::BigInt<32>;

namespace testing {
    template<>
    std::string PrintToString<BI32>(const BI32& bi) {
        std::stringstream ss;
        ss << bi.words_count << ' ' << (bi.is_positive ? '+' : '-');
        for (size_t i = 0; i < bi.words_count; ++i) {
            ss << ' ' << bi.binary[bi.words_count - 1 - i];
        }
        return std::move(ss).str();
    }
}

struct TestBigInt : algo::testing::Randomizer {

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

};

TEST_F(TestBigInt, Add) {
    {
        BI32 bi = (1ull << 33) - 1;
        ASSERT_EQ(bi - BI32(1ull << 32), (1ull << 32) - 1);
    }

    SetSeed(1);
    for (size_t i = 0; i < 10000; ++i) {
        std::string lhs = RandomBinary(RandomInt(12, 1023));
        std::string rhs = RandomBinary(RandomInt(12, 1023));
        std::string sum = NaiveAdd(lhs, rhs);

        BI32 lhs_bi {lhs};
        BI32 rhs_bi {rhs};
        BI32 sum_bi {sum};

        ASSERT_EQ(lhs_bi + rhs_bi, sum_bi);
        ASSERT_EQ(sum_bi - lhs_bi, rhs_bi);
        ASSERT_EQ(sum_bi - rhs_bi, lhs_bi);
        ASSERT_EQ(lhs_bi - sum_bi, -rhs_bi);
        ASSERT_EQ(rhs_bi - sum_bi, -lhs_bi);
    }
}

TEST_F(TestBigInt, Mul) {
    {
        BI32 bi = (1ull << 33) - 1;
        ASSERT_EQ(bi * 3ull, ((1ull << 33) - 1) * 3ull);
    }

    SetSeed(2);
    for (size_t i = 0; i < 100; ++i) {
        std::string lhs = RandomBinary(RandomInt(48, 53));
        std::string rhs = RandomBinary(RandomInt(8, 10));
        std::string mul = NaiveMul(lhs, rhs);

        BI32 lhs_bi {lhs};
        BI32 rhs_bi {rhs};
        BI32 mul_bi {mul};

        ASSERT_EQ(lhs_bi * rhs_bi, mul_bi);
        ASSERT_EQ(mul_bi / lhs_bi, rhs_bi);
        ASSERT_EQ(mul_bi / rhs_bi, lhs_bi);
    }

    for (size_t i = 0; i < 100; ++i) {
        std::string lhs = RandomBinary(RandomInt(20, 512));
        std::string rhs = RandomBinary(RandomInt(28, 512));
        std::string mul = NaiveMul(lhs, rhs);

        BI32 lhs_bi {lhs};
        BI32 rhs_bi {rhs};
        BI32 mul_bi {mul};

        ASSERT_EQ(lhs_bi * rhs_bi, mul_bi);
        ASSERT_EQ(rhs_bi * lhs_bi, mul_bi);
        ASSERT_EQ(mul_bi / lhs_bi, rhs_bi);
        ASSERT_EQ(mul_bi / rhs_bi, lhs_bi);
    }

    for (size_t i = 0; i < 100; ++i) {
        std::string lhs = RandomBinary(RandomInt(500, 512));
        std::string rhs = RandomBinary(RandomInt(500, 512));
        std::string mul = NaiveMul(lhs, rhs);

        BI32 lhs_bi {lhs};
        BI32 rhs_bi {rhs};
        BI32 mul_bi {mul};

        ASSERT_EQ(lhs_bi * rhs_bi, mul_bi);
        ASSERT_EQ(rhs_bi * lhs_bi, mul_bi);
        ASSERT_EQ(mul_bi / lhs_bi, rhs_bi);
        ASSERT_EQ(mul_bi / rhs_bi, lhs_bi);
        ASSERT_EQ(lhs_bi / mul_bi, 0);
        ASSERT_EQ(mul_bi % lhs_bi, 0);
    }

    for (size_t i = 0; i < 100; ++i) {
        std::string q = RandomBinary(RandomInt(500, 500));
        std::string d = RandomBinary(RandomInt(400, 400));
        std::string r = RandomBinary(RandomInt(100, 399));
        std::string m = NaiveAdd(NaiveMul(q, d), r);

        BI32 m_bi {m};
        BI32 d_bi {d};

        ASSERT_EQ(m_bi / d_bi, BI32{q});
        ASSERT_EQ(m_bi % d_bi, BI32{r});
    }
}

TEST(BigInt, BigInt) {
    {
        BI32 a {"0b11101100"};
        ASSERT_EQ(a, 0b11101100);
        BI32 b {"0b1001" "11110000"
                         "10100000"
                         "11110000"
                         "10100000"};
        ASSERT_EQ(b, 0b1001'1111'0000'1010'0000'1111'0000'1010'0000ull);
    }

    {
        ASSERT_EQ(BI32{"0b110111101011110011110101110111001001001"}, BI32{478326484553ull});
    }

    {
        BI32 a {12345};
        ASSERT_EQ(a.ToString(10), "12345");
        ASSERT_EQ(a.ToString(36), "9IX");
        ASSERT_EQ(BI32{478326484553ull}.ToString(10), "478326484553");

        ASSERT_EQ(BI32{"729850759413822409864896864703110513397153737649725935393728469480268371494607235845025611298670241485102205454354037818905499"}.ToString(),
                       "729850759413822409864896864703110513397153737649725935393728469480268371494607235845025611298670241485102205454354037818905499");
    }
}

void PowerModulo(const BI32& base, BI32& res, BI32&& exp, const BI32& mod) {
    if (exp.IsZero()) {
        res = 1;
    } else if (exp == 1) {
        if (base >= mod) {
            res = base % mod;
        } else {
            res = base;
        }
    } else {
        bool is_odd = exp % 2ull == 1ull;
        PowerModulo(base, res, std::move(exp >>= 1), mod);
        if (res.words_count * 2 > 32) {
            algo::BigInt<64> tmp {res.ToView()};
            tmp *= tmp;
            tmp %= algo::BigInt<64>{mod.ToView()};
            res = BI32{mod.ToView()};
        } else {
            res *= res;
            res %= mod;
        }

        if (is_odd) {
            if (base.words_count + res.words_count > 32) {
                algo::BigInt<64> tmp {res.ToView()};
                tmp *= algo::BigInt<64>{base.ToView()};
                tmp %= algo::BigInt<64>{mod.ToView()};
                res = BI32{mod.ToView()};
            } else {
                res *= base;
                res %= mod;
            }
        }
    }
}

TEST_F(TestBigInt, Algebra) {
    // https://oeis.org/A000043
    BI32 big_prime {1};
    big_prime <<= 606;
    big_prime -= 1;

    for (size_t i = 2; i < 1000; ++i) {
        BI32 base {i};
        BI32 res;
        PowerModulo(base, res, big_prime - 1ull, big_prime);
        ASSERT_EQ(res, 1);
    }
}
