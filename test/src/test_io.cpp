#include <gtest/gtest.h>

TEST(io, CheckValues)
{
  //ASSERT_EQ(tmp::add(1, 2), 3);
  EXPECT_TRUE(true);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
