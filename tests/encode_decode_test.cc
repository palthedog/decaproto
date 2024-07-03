#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/decoder.h"
#include "decaproto/encoder.h"
#include "decaproto/stream/stl.h"
#include "tests/simple.pb.h"

using namespace std;
using namespace decaproto;

float EPSILON = 0.0001;

TEST(EncodeDecodeTest, SimplTest) {
    stringstream ss;
    StlInputStream iss(&ss);
    StlOutputStream oss(&ss);

    SimpleMessage src;
    src.set_num(1234567890);
    src.set_str("Udong");
    src.set_enum_value(SimpleEnum::ENUM_B);
    src.mutable_other()->set_other_num(987654321);
    src.set_float_value(3.14);
    src.set_double_value(2.71828);

    size_t size;
    EXPECT_TRUE(EncodeMessage(oss, src, size));

    SimpleMessage dst;

    // Compare with the default value.
    EXPECT_NE(src.num(), dst.num());

    EXPECT_TRUE(DecodeMessage(iss, &dst));

    // Decode the message from the stream and compare it with the original
    // message.
    EXPECT_EQ(src.num(), dst.num());
    EXPECT_EQ(src.str(), dst.str());
    EXPECT_EQ(src.enum_value(), dst.enum_value());
    EXPECT_EQ(src.other().other_num(), dst.other().other_num());
    EXPECT_NEAR(src.float_value(), dst.float_value(), EPSILON);
    EXPECT_NEAR(src.double_value(), dst.double_value(), EPSILON);
}
