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
        return;
        ASSERT_EQ(sum_bi - lhs_bi, rhs_bi);
        ASSERT_EQ(sum_bi - rhs_bi, lhs_bi);
    }
}

TEST_F(TestBigInt, Mul) {
    {
        BI32 bi = (1ull << 33) - 1;
        ASSERT_EQ(bi * 3ull, ((1ull << 33) - 1) * 3ull);
    }

    SetSeed(2);
    for (size_t i = 0; i < 100; ++i) {
        std::string lhs = RandomBinary(RandomInt(20, 512));
        std::string rhs = RandomBinary(RandomInt(28, 512));
        std::string mul = NaiveMul(lhs, rhs);

        BI32 lhs_bi {lhs};
        BI32 rhs_bi {rhs};
        BI32 mul_bi {mul};

        ASSERT_EQ(lhs_bi * rhs_bi, mul_bi);
        ASSERT_EQ(rhs_bi * lhs_bi, mul_bi);
        //ASSERT_EQ(mul_bi / lhs_bi, rhs_bi);
        //ASSERT_EQ(mul_bi / rhs_bi, lhs_bi);
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

        //BI32 c {"12345678"};
        //ASSERT_EQ(c, 12345678);
    }

    {
        BI32 a {"0b111111111111111111111111111111111111111111111111111111111111"};
        BI32 b {"0b100000000000000000000000000000000000000000000000000000000000"};
        BI32 c {"0b100000000000000000000000000011111111111111111111111111111111"};
    }
}

    //{
        //std::string s = "92387645823763198673492876134987126489346791287631349786458971236934867";
        //BigInt<1024> a{s};
        //a.PrintDecimal();
    //}

    //{
        //BigInt<1> a {13};
        //ASSERT_TRUE(a == 13);
        //ASSERT_TRUE(a != 14);

        //ASSERT_TRUE(a < BigInt<1>{16});
        //ASSERT_TRUE(13 <= a);
        //ASSERT_TRUE(a + a == 26);
        //ASSERT_TRUE(a - a == 0);
        //ASSERT_TRUE(a * a == 169);
        //ASSERT_TRUE(a / a == 1);
        //ASSERT_TRUE(a % a == 0);
    //}
//}

//TEST(BigInt, Shift) {
    //{
        //ASSERT_TRUE((BigInt<1>{0b1111'0000'1111'0000'1111'0000'1111'0000} << 3) == 0b1'0000'1111'0000'1111'0000'1111'0000'000);
        //ASSERT_TRUE((BigInt<1>{0b1111'0000'11} >> 3) == 0b0001'1110'00);
    //}

    //{
        //ASSERT_TRUE((BigInt<2>{123456789987654321} >> 48) == (123456789987654321ull >> 48));
        //ASSERT_TRUE((BigInt<2>{123456789987654321} >> 45 << 48) == (123456789987654321ull >> 45 << 48));
    //}

    //{
        //BigInt<3> a{1};
        //a <<= 32;
        //ASSERT_TRUE(a == (1ull << 32));
        //a >>= 32;
        //ASSERT_TRUE(a == 1);
        //a >>= 1;
        //ASSERT_TRUE(a == 0);
    //}

    //{
        //BigInt<1> a {1};
        //a <<= 31;
        //a <<= 1;
        //ASSERT_TRUE(a == 0);
    //}

    //{
        //BigInt<2> a {1ull << 31};
        //ASSERT_TRUE((a << 1) == (1ull << 32));
    //}
//}

//TEST(BigInt, Arithmetic) {

//}

//TEST(BigInt, AddSub) {
    //{
        //ASSERT_TRUE(BigInt<1>{1} + 0 == 1);
        //ASSERT_TRUE(BigInt<1>{0} + 1 == 1);
        //ASSERT_TRUE(BigInt<1>{0} + 0 == 0);

        //ASSERT_TRUE(BigInt<1>{1} - 1 == 0);
        //ASSERT_TRUE(BigInt<1>{1} - 0 == 1);
        //ASSERT_TRUE(BigInt<1>{0} - 0 == 0);

        //ASSERT_TRUE(BigInt<1>{0} - 1 == -1);
        //ASSERT_TRUE(BigInt<1>{-1} - -1 == 0);
    //}

    //{
        //BigInt<2> a = (1ull << 32) - 1;
        //a += 1;
        //ASSERT_TRUE(a == 1ull << 32);
        //a -= 1;
        //ASSERT_TRUE(a == (1ull << 32) - 1);
    //}

    //{
        //BigInt<96> a = 1;
        //a <<= 32;
        //a += (1ull << 32) - 1;
        //a <<= 32;
        //a += (1ull << 32) - 1;

        //ASSERT_TRUE(a + 1 == (BigInt<96>{1} << 65));
    //}
    
    //{
        //int64_t count = 0;
        //BigInt<2> addable;

        //for (int64_t i = 0; i < 100000; ++i) {
            //addable += i;
            //count += i;
            //ASSERT_TRUE(addable == count) << "Iteration " << i << ", count: " << count;
        //}

        //for (int64_t i = 100'000; i < 123'123; ++i) {
            //addable -= i;
            //count -= i;
            //ASSERT_TRUE(addable == count) << "Iteration " << i << ", count: " << count;
        //}
    //}

    //{
        //int64_t count = 0;
        //int64_t count2 = 0;
        //BigInt<65> a, b;
        //for (int64_t i : {23654432453, 212398345434324, -23424523456245, 345126245622341, -234245135425, -3123324213451231, 534264531232346, -12895343422145}) {
            //a += i;
            //count += i;
            //ASSERT_TRUE(a == count) << i;

            //b -= i;
            //count2 -= i;
            //ASSERT_TRUE(b == count2) << i;
        //}
    //}
//}

//TEST(BigInt, Mul) {
    //{
        //BigInt<1024> a = 2;
        //a = a.Power(2000) + a.Power(1876) + 917397468272;

        //BigInt<1024> b = 15;
        //b = b.Power(123) + b.Power(89) + 912351273312;
        //b *= b;

        //ASSERT_TRUE(a * b == b * a);
    //}

    //{
        //ASSERT_TRUE(BigInt<1>{12145} * 34324 == 12145 * 34324);
        //ASSERT_TRUE(BigInt<1>{12145} * -34324 == -12145 * 34324);
        //ASSERT_TRUE(BigInt<1>{0b1111} * 0b1'0000 == 0b1111'0000);
        //ASSERT_TRUE(BigInt<1>{0b0'1111'1111'1111'1111} * 0b10 == 0b1'1111'1111'1111'1110);
    //}

    //{
        //BigInt<2> a = 13;
        //for (int64_t i : {1ll << 32, 1ll << 41, 1ll << 17, 1ll << 58}) {
            //ASSERT_TRUE(a * i == 13ll * i) << i;
        //}
    //}

    //{
        //BigInt<32> asd = 1234343244;
        //asd *= 123412423523423123;
        //BigInt<32> tmp = 0;
        //int64_t limit = 286'325;
        //for (int64_t i = 0; i < limit; ++i) {
            //tmp += asd;
        //}

        //ASSERT_TRUE(tmp == asd * limit);
    //}

    //{
        //BigInt<32> asd = 1;
        //for (int i = 0; i < 1023; ++i) {
            //asd = asd + asd;
        //}
        //ASSERT_TRUE(asd == (BigInt<32>{1} << 1023));
        //ASSERT_TRUE(asd == BigInt<32>{2}.Power(1023));
    //}

    //{
        //BigInt<32> lhs = {
            //"0b11010101001010101001010101001010101001010101001010101010010101010010"
              //"101010010101010100101011101111011010010101001010011101010010001010101"
        //};

        //BigInt<32> rhs = {
            //"0b11100100111010101000100000101000001001110110101001111101110011010101"
              //"00011001101001101100011100010100101000110010101111000100110101010000"
              //"01010100001010101010111011111100011110101001111010001101100111011001"
              //"1110001011100"
        //};

        //BigInt<32> res = {
            //"0b10111110100111010011011101000011000000000100000111100001111000100011"
              //"11010011011111100000111011101110010001010110100110101011111110111101"
              //"11011000001100011111011111101111100111000010110011100110011101100101"
              //"01110011010100000100111110101000110111101100011110001101000111000100"
              //"00110011101000110001101110010111100010111001010001000111010101110111"
              //"11101010001100"
        //};

        //for (int i = 0; i < 100000; ++i) {
            //ASSERT_TRUE(lhs * rhs == res);
        //}
    //}


    //{
        //BigInt<1024> lhs = {
            //"0b11010101001010101001010101001010101001010101001010101010010101010010"
              //"101010010101010100101011101111011010010101001010011101010010001010101"
        //};

        //BigInt<1024> rhs = {
            //"0b11100100111010101000100000101000001001110110101001111101110011010101"
              //"00011001101001101100011100010100101000110010101111000100110101010000"
              //"01010100001010101010111011111100011110101001111010001101100111011001"
              //"1110001011100"
        //};

        //BigInt<1024> res = {
            //"0b10111110100111010011011101000011000000000100000111100001111000100011"
              //"11010011011111100000111011101110010001010110100110101011111110111101"
              //"11011000001100011111011111101111100111000010110011100110011101100101"
              //"01110011010100000100111110101000110111101100011110001101000111000100"
              //"00110011101000110001101110010111100010111001010001000111010101110111"
              //"11101010001100"
        //};

        //for (int i = 0; i < 1000; ++i) {
            //lhs *= 2;
            //res *= 2;
            //ASSERT_TRUE(lhs * rhs == res);
        //}
    //}
//}

//TEST(BigInt, Div) {
    //{
        //BigInt<4> a {3746123786218736ul};
        //ASSERT_TRUE((a >> 1) == (a / 2));
        //ASSERT_TRUE(a % 2 == 0);
        //ASSERT_TRUE((a + 1) % 2 == 1);
    //}

    //{
        //ASSERT_TRUE(BigInt<1>{3} / 10 == 0);        

        //int64_t a = 386273648273648432ll;
        //BigInt<2> b {a};
        //while (a != 0) {
            //ASSERT_TRUE(b % 10 == a % 10);
            //b /= 10;
            //a /= 10;
        //}
        //ASSERT_TRUE(b.IsZero());
    //}

    //{
        //int64_t h = 2143523;
        //int64_t l = 123245;
        //int64_t exp = 1ull << 32;
        //BigInt<2> a {h * exp + l};
        //ASSERT_TRUE(a / exp == h);
        //ASSERT_TRUE(a % exp == l);
    //}

    //{
        //BigInt<1024> a {2};
        //a = a.Power(900);
        //ASSERT_TRUE(a / BigInt<1024>{2}.Power(880) == BigInt<1024>{2}.Power(20));
    //}

    //{
        //int64_t divisor = 327897249821273987ull;
        //int64_t divident = 123495ull;

        //BigInt<2> a = divisor;
        //ASSERT_TRUE(a / divident == divisor / divident);
        //ASSERT_TRUE(a % divident == divisor % divident);
    //}

    //{
        //for (auto [divisor, divident] : {std::tuple{99263870170498186ll, 480852372},
                                                   //{-199503489089220644ll, 407779708},
                                                   //{587144900876542807ll, 949756056},
                                                   //{-297572886055734023ll, 936295942},
                                                   //{367109822501180285ll, 621679020}})
        //{
            //BigInt<2> a = divisor;
            //ASSERT_TRUE(a / divident == divisor / divident) << divisor << '\t' << divident;
            //ASSERT_TRUE(a % divident == divisor % divident) << divisor << '\t' << divident;
        //}
    //}

    //{
        //int64_t divisor = 211'742'143'594'981'396ll; 
        //int64_t divident = 98'131'191'013'928'430ll;
        //BigInt<2> a = divisor;
        //ASSERT_TRUE(a / divident == divisor / divident);
        //ASSERT_TRUE(a % divident == divisor % divident);
    //}

    //{
        //for (auto [divisor, divident] : {std::pair{211'742'143'594'981'396ll, 98'131'191'013'928'430ll},
                                                  //{453793416944131859ll, 35353803976864009ll},
                                                  //{644089935536868565ll, 379814261487877315ll},
                                                  //{232814861694501940ll, 146074081406853314ll},
                                                  //{940219789698254288ll, 50583654315824559ll}})
        //{
            //BigInt<2> a = divisor;
            //ASSERT_TRUE(a / divident == divisor / divident) << divisor << '\t' << divident << '\t' << divisor / divident;
            //ASSERT_TRUE(a % divident == divisor % divident) << divisor << '\t' << divident;
        //}
    //}

    //{
        //BigInt<8> a {"0b11100100011010101100110111111011"
                       //"00110000111010100000100100101110"
                       //"00001100100110101011101111100100"
                       //"10010001010001000110101100011101"
                       //"0001100100101110110010"};
        //BigInt<8> b {"0b11000001011000000001011111010100"
                       //"11000000100000011000100011000101"
                       //"01110100111111000"};

        //BigInt<8> q {"0b10010111001100011111000001010101"
                       //"01010100101001010100111001110001"
                       //"010010"};

        //BigInt<8> r {"0b11101010110110111101011111111000"
                       //"01010001001001111101110110011000"
                       //"011101001000010"};

        //ASSERT_TRUE(a / b == q);
        //ASSERT_TRUE(a % b == r);
    //}

    //{
        //using T = BigInt<1024>;
        //T a = 2;
        //a = a.Power(2000) + a.Power(1876) + 917397468272;

        //T b = 15;
        //b = b.Power(123) + b.Power(89) + 912351273312;
        //b *= b;

        //T q = a / b;
        //T r = a % b;
        //ASSERT_TRUE(a == b * q + r);
    //}
//}

//TEST(BigInt, Bits) {
    //{
        //int i = 1234321123;
        //BigInt<32> a {i};
        //for (int j : {21345, 123245, 567341234, 123432}) {
            //ASSERT_TRUE((i & j) == (a & j)) << j;
            //ASSERT_TRUE((i | j) == (a | j)) << j;
            //ASSERT_TRUE((i ^ j) == (a ^ j)) << j;
        //}
    //}

    //{
        //BigInt<123> a {1};
        //a <<= 100;
        //ASSERT_TRUE((a & (a - 1)) == 0);

        //a += 12325354312;
        //ASSERT_FALSE((a & (a - 1)) == 0);
    //}
//}

//TEST(BigInt, File) {
    //std::ifstream inp {"test_data.txt"};

    //auto read = [&](std::string& x) {
        //inp >> x;
        //if (x[0] == '-') {
            //x = "-0b" + x.substr(1);
        //} else {
            //x = "0b" + x;
        //}
    //};
    //for (size_t i = 0; i < 100; ++i) {
        //std::string x, y, add, sub, mul, div, mod;
        //read(x);
        //read(y);
        //read(add);
        //read(sub);
        //read(mul);
        //read(div);
        //read(mod);
        //BigInt<5> lhs {x};
        //BigInt<5> rhs{y};
        //BigInt<5> radd{add};
        //BigInt<5> rsub{sub};
        //BigInt<5> rmul{mul};
        //BigInt<5> rdiv{div};
        //BigInt<5> rmod{mod};

        //ASSERT_TRUE(lhs + rhs == radd) << "Line: " << i;
        //ASSERT_TRUE(lhs - rhs == rsub) << "Line: " << i;
        //ASSERT_TRUE(lhs * rhs == rmul) << "Line: " << i;
        //ASSERT_TRUE(lhs / rhs == rdiv) << "Line: " << i;
        //ASSERT_TRUE(lhs % rhs == rmod) << "Line: " << i;
    //}
//}

//TEST(BigInt, Algebra) {
    //// https://oeis.org/A000043
    //BigInt<2> bigPrime = BigInt<2>{2}.Power(61) - 1;
    //for (int i = 2; i < 1000; ++i) {
        //BigInt<2> pow = BigInt<2>{i}.Power(bigPrime - 1, bigPrime);
        //ASSERT_TRUE(pow == 1) << i;
    //}
//}
