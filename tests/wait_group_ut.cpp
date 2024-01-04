#include <gtest/gtest.h>

import algo;

TEST(WaitGroup, Trivial) {
  algo::WaitGroup wg;
  ASSERT_TRUE(wg.TryInc());
  ASSERT_TRUE(wg.TryDec());
  ASSERT_FALSE(wg.TryDec());
  ASSERT_TRUE(wg.TryInc());
  wg.Block();
  ASSERT_FALSE(wg.TryInc());
  ASSERT_TRUE(wg.Dec());
  ASSERT_FALSE(wg.Dec());
  wg.Wait();
}
