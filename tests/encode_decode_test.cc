#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/decoder.h"
#include "decaproto/encoder.h"
#include "decaproto/stream/stl.h"
#include "tests/simple.pb.h"

using namespace std;
using namespace decaproto;

float EPSILON = 0.0001;

TEST(EncodeDecodeTest, SimpleMessageTest) {
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
    src.set_bool_value(true);
    src.set_bool_value(false);

    size_t size;
    // Encode the message to the stream.
    EXPECT_TRUE(EncodeMessage(oss, src, size));

    // Decode the message from the stream.
    SimpleMessage dst;
    EXPECT_TRUE(DecodeMessage(iss, &dst));

    // Let's compare the fields of the source and destination messages.
    EXPECT_EQ(src.num(), dst.num());
    EXPECT_EQ(src.str(), dst.str());
    EXPECT_EQ(src.enum_value(), dst.enum_value());
    EXPECT_EQ(src.other().other_num(), dst.other().other_num());
    EXPECT_NEAR(src.float_value(), dst.float_value(), EPSILON);
    EXPECT_NEAR(src.double_value(), dst.double_value(), EPSILON);
    EXPECT_EQ(src.bool_value(), dst.bool_value());
}
