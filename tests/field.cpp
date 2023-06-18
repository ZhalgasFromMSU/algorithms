#include <algo/finite_field/field.hpp>

#include <gtest/gtest.h>

using namespace algo;

TEST(Field, BigInt) {
    ModuloField<BigInt<4>, BigInt<4>{13}, BigInt<4>{2}> f {2};
    f.Reversed().value.PrintWords();
    ASSERT_TRUE(f.Reversed() == 7);
}
