#include <gtest/gtest.h>

#include "tests/def_order.pb.h"

// What if a depending message is defined before a depended message?
// See `ef_order.proto` for the definition order.
TEST(DefinitionalOrderTest, SimpleTest) {
    DependingMessage m;

    m.mutable_depended()->set_num(100);
    m.set_depended_enum(DependedEnum::DEP_ENUM_A);

    EXPECT_EQ(100, m.depended().num());
    EXPECT_EQ(DependedEnum::DEP_ENUM_A, m.depended_enum());
}
