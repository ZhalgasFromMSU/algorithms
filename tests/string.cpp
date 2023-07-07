#include <algo/string.hpp>

#include <gtest/gtest.h>

using namespace algo;

TEST(String, Palindrome) {
    //ASSERT_EQ(*String::MaxPalindromes("babad"), std::vector<size_t>({1, 3, 3, 1, 1, 1}));
    //ASSERT_EQ(*String::MaxPalindromes("cbbd"), std::vector<size_t>({1, 2, 1, 1, 1}));

    //ASSERT_EQ(*String::MaxPalindromes("bb"), std::vector<size_t>({2, 1, 1}));


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

    //const char s[] = "01232100123210";
    //ASSERT_EQ(*String::MaxPalindromes(std::string_view{s}), naive(s));

    const char s2[] = "aaaaaa";
    ASSERT_EQ(*String::MaxPalindromes(std::string_view{s2}), naive(s2)) << s2;

    const char s3[] = "aaaaaaa";
    ASSERT_EQ(*String::MaxPalindromes(std::string_view{s3}), naive(s3)) << s3;
}
