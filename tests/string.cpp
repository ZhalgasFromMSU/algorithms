#include <algo/string.hpp>

#include <gtest/gtest.h>

using namespace algo;

TEST(String, Palindrome) {
    ASSERT_EQ(*String::MaxPalindromes("babad"), std::vector<size_t>({1, 3, 3, 1, 1, 1}));
    ASSERT_EQ(*String::MaxPalindromes("cbbd"), std::vector<size_t>({1, 2, 1, 1, 0}));
}
