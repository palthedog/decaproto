#include <gtest/gtest.h>

#include "tests/simple.pb.h"

TEST(SimpleTest, NumTest) {
  SimpleMessage m;

  EXPECT_NE(100, m.num());
  m.set_num(100);
  EXPECT_EQ(100, m.num());

  m.clear_num();
  EXPECT_FALSE(m.has_num());
}

TEST(SimpleTest, StrTest) {
  SimpleMessage m;

  EXPECT_NE("foo bar", m.str());
  m.set_str("foo bar");
  EXPECT_EQ("foo bar", m.str());

  m.clear_str();
  EXPECT_FALSE(m.has_str());
}

TEST(SimpleTest, EnumTest) {
  SimpleMessage m;

  // EXPECT_NE("foo bar", m.str());
  EXPECT_NE(SimpleEnum::ENUM_A, m.enum_value());
  m.set_enum_value(SimpleEnum::ENUM_A);
  EXPECT_EQ(SimpleEnum::ENUM_A, m.enum_value());

  m.clear_enum_value();
  EXPECT_FALSE(m.has_enum_value());
}

TEST(SimpleTest, MessageTest) {
  SimpleMessage m;

  EXPECT_FALSE(m.has_other());
  OtherMessage* mut_other = m.mutable_other();
  mut_other->set_other_num(100);

  EXPECT_TRUE(m.has_other());
  EXPECT_EQ(100, mut_other->other_num());
  EXPECT_EQ(100, m.other().other_num());

  m.clear_other();
  EXPECT_FALSE(m.has_other());
}
