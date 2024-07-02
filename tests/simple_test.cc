#include <gtest/gtest.h>

#include "tests/simple.pb.h"

TEST(SimpleTest, NumTest) {
    SimpleMessage m;

    // Default value
    EXPECT_EQ(0, m.num());

    m.set_num(100);
    EXPECT_EQ(100, m.num());

    m.clear_num();
    EXPECT_EQ(0, m.num());
}

TEST(SimpleTest, StrTest) {
    SimpleMessage m;

    // Default value
    EXPECT_EQ("", m.str());

    m.set_str("foo bar");
    EXPECT_EQ("foo bar", m.str());

    m.clear_str();
    EXPECT_EQ("", m.str());
}

TEST(SimpleTest, EnumTest) {
    SimpleMessage m;

    // Default value
    EXPECT_EQ(0, m.enum_value());

    EXPECT_NE(SimpleEnum::ENUM_A, m.enum_value());
    m.set_enum_value(SimpleEnum::ENUM_A);
    EXPECT_EQ(SimpleEnum::ENUM_A, m.enum_value());

    m.clear_enum_value();
    EXPECT_EQ(0, m.enum_value());
}

TEST(SimpleTest, MessageTest) {
    SimpleMessage m;

    // Default value is not set
    EXPECT_FALSE(m.has_other());

    OtherMessage* mut_other = m.mutable_other();
    mut_other->set_other_num(100);

    EXPECT_TRUE(m.has_other());
    EXPECT_EQ(100, mut_other->other_num());
    EXPECT_EQ(100, m.other().other_num());

    m.clear_other();
    EXPECT_FALSE(m.has_other());
}

TEST(SimpleTest, SubMessagePresenceTest) {
    SimpleMessage m;

    EXPECT_FALSE(m.has_other());

    // getter returns a default value
    const OtherMessage& other = m.other();
    EXPECT_EQ(0, other.other_num());

    // but still its hasser should return false because it's not set
    EXPECT_FALSE(m.has_other());

    m.mutable_other();
    // Once we call mutable_other, its hasser should return true even if
    // sub-message's field is not set
    EXPECT_TRUE(m.has_other());

    m.clear_other();
    EXPECT_FALSE(m.has_other());
}
