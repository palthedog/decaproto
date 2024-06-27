#include <gtest/gtest.h>

#include "tests/repeated.pb.h"

TEST(RepeatedTest, NumTest) {
  RepeatedMessage m;

  EXPECT_EQ(0, m.nums().size());
  auto mut = m.mutable_nums();
  mut->push_back(1);
  mut->push_back(2);
  mut->push_back(3);
  EXPECT_EQ(3, m.nums().size());

  m.clear_nums();
  EXPECT_EQ(0, m.nums().size());
}

TEST(RepeatedTest, StringTest) {
  RepeatedMessage m;

  EXPECT_EQ(0, m.strs().size());
  auto mut = m.mutable_strs();
  mut->push_back("1");
  mut->push_back("2");
  mut->push_back("3");
  EXPECT_EQ(3, m.strs().size());

  m.clear_strs();
  EXPECT_EQ(0, m.strs().size());
}

TEST(RepeatedTest, EnumTest) {
  RepeatedMessage m;

  EXPECT_EQ(0, m.enum_values().size());
  auto mut = m.mutable_enum_values();
  mut->push_back(RepeatedEnum::REP_ENUM_A);
  mut->push_back(RepeatedEnum::REP_ENUM_B);
  mut->push_back(RepeatedEnum::REP_ENUM_C);
  EXPECT_EQ(3, m.enum_values().size());

  m.clear_enum_values();
  EXPECT_EQ(0, m.enum_values().size());
}
