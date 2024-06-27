#include <gtest/gtest.h>

#include "tests/simple.pb.h"

TEST(SimpleTest, NumTest) {
  SimpleMessage m;

  EXPECT_NE(100, m.num());
  m.set_num(100);
  EXPECT_EQ(100, m.num());
}

TEST(SimpleTest, StrTest) {
  SimpleMessage m;

  EXPECT_NE("foo bar", m.str());
  m.set_str("foo bar");
  EXPECT_EQ("foo bar", m.str());
}
