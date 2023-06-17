#include <algo/finite_field/bigint.hpp>

#include <gtest/gtest.h>

using namespace algo;

TEST(BigInt, BigInt) {
    {
        BigInt<27> a {"0b11101100"};
        ASSERT_TRUE(a == 0b11101100);
        BigInt<49> b {"0b1'1111'0000'1010'0000'1111'0000'1010'0000"};
        ASSERT_TRUE(b == 0b1'1111'0000'1010'0000'1111'0000'1010'0000ull);
    }

    {
        BigInt<32> a {13};
        ASSERT_TRUE(a == 13);
        ASSERT_TRUE(a != 14);

        ASSERT_TRUE(a < BigInt<32>{16});
        ASSERT_TRUE(13 <= a);
        ASSERT_TRUE(a + a == 26);
        ASSERT_TRUE(a - a == 0);
        ASSERT_TRUE(a * a == 169);
        ASSERT_TRUE(a / a == 1);
        ASSERT_TRUE(a % a == 0);
    }
}

TEST(BigInt, Shift) {
    {
        ASSERT_TRUE((BigInt<10>{0b1111'0000'11} << 3) == 0b1000'0110'00);
        ASSERT_TRUE((BigInt<10>{0b1111'0000'11} >> 3) == 0b0001'1110'00);
    }

    {
        ASSERT_TRUE((BigInt<64>{123456789987654321} >> 48) == (123456789987654321ull >> 48));
        ASSERT_TRUE((BigInt<64>{123456789987654321} >> 45 << 48) == (123456789987654321ull >> 45 << 48));
    }

    {
        BigInt<96> a{1};
        a <<= 32;
        ASSERT_TRUE(a == (1ull << 32));
        a >>= 32;
        ASSERT_TRUE(a == 1);
    }
}

TEST(BigInt, AddSub) {
    {
        ASSERT_TRUE(BigInt<1>{1} + 0 == 1);
        ASSERT_TRUE(BigInt<1>{0} + 1 == 1);
        ASSERT_TRUE(BigInt<1>{0} + 0 == 0);

        ASSERT_TRUE(BigInt<1>{1} - 1 == 0);
        ASSERT_TRUE(BigInt<1>{1} - 0 == 1);
        ASSERT_TRUE(BigInt<1>{0} - 0 == 0);

        ASSERT_TRUE(BigInt<1>{0} - 1 == -1);
        ASSERT_TRUE(BigInt<1>{-1} - -1 == 0);
    }

    {
        BigInt<64> a = (1ull << 32) - 1;
        a += 1;
        ASSERT_TRUE(a == 1ull << 32);
        a -= 1;
        ASSERT_TRUE(a == (1ull << 32) - 1);
    }

    {
        BigInt<96> a = 1;
        a <<= 32;
        a += (1ull << 32) - 1;
        a <<= 32;
        a += (1ull << 32) - 1;

        ASSERT_TRUE(a + 1 == (BigInt<96>{1} << 65));
    }
    
    {
        int64_t count = 0;
        BigInt<64> addable;

        for (int64_t i = 0; i < 100000; ++i) {
            addable += i;
            count += i;
            ASSERT_TRUE(addable == count) << "Iteration " << i << ", count: " << count;
        }

        for (int64_t i = 100'000; i < 123'123; ++i) {
            addable -= i;
            count -= i;
            ASSERT_TRUE(addable == count) << "Iteration " << i << ", count: " << count;
        }
    }

    {
        int64_t count = 0;
        int64_t count2 = 0;
        BigInt<65> a, b;
        for (int64_t i : {23654432453, 212398345434324, -23424523456245, 345126245622341, -234245135425, -3123324213451231, 534264531232346, -12895343422145}) {
            a += i;
            count += i;
            ASSERT_TRUE(a == count) << i;

            b -= i;
            count2 -= i;
            ASSERT_TRUE(b == count2) << i;
        }
    }
}

TEST(BigInt, Mul) {
    {
        ASSERT_TRUE(BigInt<32>{12145} * 34324 == 12145 * 34324);
        ASSERT_TRUE(BigInt<32>{12145} * -34324 == 12145 * 34324);
        ASSERT_TRUE(BigInt<8>{0b1111} * 0b1'0000 == 0b1111'0000);
        ASSERT_TRUE(BigInt<17>{0b0'1111'1111'1111'1111} * 0b10 == 0b1'1111'1111'1111'1110);
    }

    {
        BigInt<64> a = 13;
        for (int64_t i : {1ll << 32, 1ll << 41, 1ll << 17, 1ll << 58}) {
            ASSERT_TRUE(a * i == 13ll * i) << i;
        }
    }

    {
        BigInt<1024> asd = 1234343244;
        asd *= 123412423523423123;
        BigInt<1024> tmp = 0;
        for (int i = 0; i < 286'325; ++i) {
            tmp += asd;
        }

        ASSERT_TRUE(tmp == asd * 286'325);
    }

    {
        BigInt<1024> asd = 1;
        for (int i = 0; i < 1023; ++i) {
            asd = asd + asd;
        }
        ASSERT_TRUE(asd == (BigInt<1024>{1} << 1023));
        ASSERT_TRUE(asd == BigInt<1024>{2}.Power(1023));
    }

    {
        BigInt<1024> lhs = {
            "0b11010101001010101001010101001010101001010101001010101010010101010010"
              "101010010101010100101011101111011010010101001010011101010010001010101"
        };

        BigInt<1024> rhs = {
            "0b11100100111010101000100000101000001001110110101001111101110011010101"
              "00011001101001101100011100010100101000110010101111000100110101010000"
              "01010100001010101010111011111100011110101001111010001101100111011001"
              "1110001011100"
        };

        BigInt<1024> res = {
            "0b10111110100111010011011101000011000000000100000111100001111000100011"
              "11010011011111100000111011101110010001010110100110101011111110111101"
              "11011000001100011111011111101111100111000010110011100110011101100101"
              "01110011010100000100111110101000110111101100011110001101000111000100"
              "00110011101000110001101110010111100010111001010001000111010101110111"
              "11101010001100"
        };

        for (int i = 0; i < 10000; ++i) {
            ASSERT_TRUE(lhs * rhs == res);
        }
    }
}

TEST(BigInt, Div) {
    {
        BigInt<119> a {3746123786218736ul};
        ASSERT_TRUE((a >> 1) == (a / 2));
        ASSERT_TRUE(a % 2 == 0);
        ASSERT_TRUE((a + 1) % 2 == 1);
    }

    {
        int64_t h = 2143523;
        int64_t l = 123245;
        int64_t exp = 1ull << 32;
        BigInt<64> a {h * exp + l};
        ASSERT_TRUE(a / exp == h);
        ASSERT_TRUE(a % exp == l);
    }

    {
        BigInt<1024> a {2};
        a = a.Power(900);
        ASSERT_TRUE(a / BigInt<1024>{2}.Power(880) == BigInt<1024>{2}.Power(20));
    }
}

TEST(BigInt, Bits) {
    {
        int i = 1234321123;
        BigInt<32> a {i};
        for (int j : {21345, 123245, 567341234, 123432}) {
            ASSERT_TRUE((i & j) == (a & j)) << j;
            ASSERT_TRUE((i | j) == (a | j)) << j;
            ASSERT_TRUE((i ^ j) == (a ^ j)) << j;
        }
    }

    {
        BigInt<123> a {1};
        a <<= 100;
        ASSERT_TRUE((a & (a - 1)) == 0);

        a += 12325354312;
        ASSERT_FALSE((a & (a - 1)) == 0);
    }
}
