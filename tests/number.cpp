#include <algo/finite_field/concepts.hpp>
#include <algo/finite_field/number.hpp>

#include <gtest/gtest.h>

#include <complex>

using namespace algo;

template<Integral T>
struct S {
    T x;
};

namespace algo {
    struct MyIntegral {};

    template<>
    struct IsIntegral<MyIntegral> : std::true_type {};
} // namespace algo

TEST(Number, Concept) {
    [[maybe_unused]] S<int> s;
    [[maybe_unused]] S<MyIntegral> ss;
    static_assert(IntegralTraits<int>::kBitSize == 32);
    static_assert(IntegralTraits<MyIntegral>::kBitSize == 8);
}

TEST(Number, BigInt) {
    BigInt<32> a {13};
    ASSERT_TRUE(a == 13);
    ASSERT_TRUE(a != 14);

    ASSERT_TRUE(a < BigInt<32>{16});
    ASSERT_TRUE(13 <= a);

    BigInt<29> shift {0b1000'0101};
    shift <<= 1;
    ASSERT_TRUE(shift == 0b01'0000'1010);
    shift <<= 13;
    ASSERT_TRUE(shift == 0b10'0001'0100'0000'0000'0000);

    shift >>= 13;
    ASSERT_TRUE(shift == 0b01'0000'1010);

    shift >>= 4;
    ASSERT_TRUE(shift == 0b1'0000);

    shift = 0b1001'1101'0000'0000;
    shift >>= 5;
    ASSERT_TRUE(shift == 0b100'1110'1000);

    shift = 0b1001'1001'0000'0000'0000'0000;
    shift >>= 17;
    ASSERT_TRUE(shift == 0b100'1100);

    BigInt<8> another_shift {0b1110'0000};
    another_shift >>= 5;
    ASSERT_TRUE(another_shift == 7);

    BigInt<13> str {"0b10011101"};
    ASSERT_TRUE(str == 0b10011101);

    BigInt<1024> b {"0b111"};
    ASSERT_TRUE(b == 7);
    b >>= 1;
    ASSERT_TRUE(b == 3);
    b <<= 1;
    ASSERT_TRUE(b == 6);

    b <<= 1021;
    b >>= 1021;
    ASSERT_TRUE(b == 6);

    b <<= 1022;
    b >>= 1022;
    ASSERT_TRUE(b == 2);

    b += 1;
    ASSERT_TRUE(b == 3);

    b += 1000000000;
    ASSERT_TRUE(b == 1000000003);

    another_shift += 18;
    ASSERT_TRUE(another_shift == 25);

    ASSERT_TRUE((BigInt<8>(0b1) << 3) == 0b1000);
    ASSERT_TRUE((BigInt<11>(0b1010) >> 1) == 0b101);

    BigInt<14> add = 0b1111'1111;
    add += 1;
    ASSERT_TRUE(add == 0b1'0000'0000);
    add -= 1;
    ASSERT_TRUE(add == 0b1111'1111);
    add -= 0b1111'1111;
    ASSERT_TRUE(add == 0);
    add -= 1;
    ASSERT_TRUE(add == -1);
    add += 13;
    ASSERT_TRUE(add == 12);
    add -= -255;
    ASSERT_TRUE(add == 267);
    add += -270;
    ASSERT_TRUE(add == -3);


    ASSERT_TRUE(BigInt<13>{13} + -26 == -13);
    ASSERT_TRUE(13 + BigInt<13>{-26} == -13);
}

TEST(BigInt, Multiplication) {
    {
        ASSERT_TRUE(BigInt<32>{12145} * 34324 == 12145 * 34324);
        ASSERT_TRUE(BigInt<32>{12145} * -34324 == 12145 * 34324);
        ASSERT_TRUE(BigInt<8>{0b1111} * 0b1'0000 == 0b1111'0000);
        ASSERT_TRUE(BigInt<17>{0b0'1111'1111'1111'1111} * 0b10 == 0b1'1111'1111'1111'1110);
    }

    {
        BigInt<1024> asd = 1234343244;
        BigInt<1024> tmp = 0;
        for (int i = 0; i < 86'325; ++i) {
            tmp += asd;
        }

        ASSERT_TRUE(tmp == asd * 86'325);
    }
}

TEST(BigInt, Division) {
    {
        std::cerr << (-1 % 3) << std::endl;
    }
}

