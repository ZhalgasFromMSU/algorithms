#include <algo/finite_field/util.hpp>

#include <gtest/gtest.h>

TEST(Multiplication, Asserts) {
    ASSERT_TRUE(algo::IsPrime(3));
    ASSERT_FALSE(algo::IsPrime(4));
}
