#include "utils.hpp"

#include <algo/string.hpp>

#include <gtest/gtest.h>

using namespace algo;

struct TestString : algo::testing::Randomizer {

};

TEST_F(TestString, Palindrome) {
    ASSERT_EQ(*String::MaxPalindromes("babad"), std::vector<size_t>({1, 3, 3, 1, 1, 1}));
    ASSERT_EQ(*String::MaxPalindromes("cbbd"), std::vector<size_t>({1, 2, 1, 1, 1}));

    ASSERT_EQ(*String::MaxPalindromes("bb"), std::vector<size_t>({2, 1, 1}));


    auto naive = [](std::string_view s) -> std::vector<size_t> {
        std::vector<size_t> ret(s.size());

        for (size_t i = 0; i < s.size(); ++i) {
            size_t odd_size = 1;
            while (i + odd_size / 2 < s.size() && i >= (odd_size - 1) / 2 && s[i - (odd_size - 1) / 2] == s[i + odd_size / 2]) {
                odd_size += 2;
            }

            size_t even_size = 2;
            while (i + even_size / 2 < s.size() && i >= (even_size - 1) / 2 && s[i - (even_size - 1) / 2] == s[i + even_size / 2]) {
                even_size += 2;
            }

            ret[i] = std::max(odd_size - 2, even_size - 2);
        }

        return ret;
    };

    for (std::string_view s : {"01232100123210", "aaaaaa", "aaaaaaa"}) {
        ASSERT_EQ(*String::MaxPalindromes(s), naive(s)) << s;
    }

    SetSeed(1);
    for (size_t i = 0; i < 100; ++i) {
        std::string s = RandomString(RandomInt(10, 1000), "ab");
        ASSERT_EQ(*String::MaxPalindromes(s), naive(s)) << s;
    }
}

TEST_F(TestString, ZFunc) {
    auto naive = [](std::string_view s) -> std::vector<size_t> {
        std::vector<size_t> ret(s.size());
        for (size_t i = 0; i < ret.size(); ++i) {
            size_t size = 0;
            while (i + size < ret.size() && s[i + size] == s[size]) {
                size += 1;
            }
            ret[i] = size;
        }
        return ret;
    };

    for (std::string_view s : {"aaa", "aaabbb"}) {
        ASSERT_EQ(*String::ZFunc(s), naive(s)) << s;
    }

    SetSeed(2);
    for (size_t i = 0; i < 100; ++i) {
        std::string s = RandomString(RandomInt(10, 1000), "ab");
        ASSERT_EQ(*String::ZFunc(s), naive(s)) << s;
    }
}
