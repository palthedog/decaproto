#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/decoder.h"
#include "decaproto/encoder.h"
#include "decaproto/stream/stl.h"
#include "tests/numeric_types.pb.h"
#include "tests/repeated.pb.h"
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

template <typename T>
void testRepeatedField(const T& src, const T& dst) {
    EXPECT_EQ(src.size(), dst.size());
    for (size_t i = 0; i < src.size(); ++i) {
        EXPECT_EQ(src[i], dst[i]);
    }
}

TEST(EncodeDecodeTest, RepeatedMessageTest) {
    stringstream ss;
    StlInputStream iss(&ss);
    StlOutputStream oss(&ss);

    RepeatedMessage src;
    src.mutable_nums()->push_back(10);
    src.mutable_nums()->push_back(20);

    src.mutable_strs()->push_back("Udong");
    src.mutable_strs()->push_back("Pow");

    src.mutable_enum_values()->push_back(REP_ENUM_A);
    src.mutable_enum_values()->push_back(REP_ENUM_B);

    std::vector<SimpleMessage>* mut_simple_messages =
            src.mutable_simple_messages();
    mut_simple_messages->resize(3);
    (*mut_simple_messages)[0].set_str("foo");
    //(*mut_simple_messages)[1]  // keep it a default message
    (*mut_simple_messages)[2].set_str("bar");

    std::vector<OtherMessage>* mut_other_messages =
            src.mutable_other_messages();
    mut_other_messages->resize(3);
    (*mut_other_messages)[0].set_other_num(100);
    // (*mut_other_messages)[1]  // keep it a default message
    (*mut_other_messages)[2].set_other_num(200);

    size_t size;

    // Encode the message to the stream.

    EXPECT_TRUE(EncodeMessage(oss, src, size));

    // Decode the message from the stream.
    RepeatedMessage dst;
    EXPECT_TRUE(DecodeMessage(iss, &dst));

    // Let's compare the fields of the source and destination messages.
    testRepeatedField(src.nums(), dst.nums());
    testRepeatedField(src.strs(), dst.strs());
    testRepeatedField(src.enum_values(), dst.enum_values());

    EXPECT_EQ(src.simple_messages().size(), dst.simple_messages().size());
    for (size_t i = 0; i < src.simple_messages().size(); ++i) {
        EXPECT_EQ(
                src.simple_messages()[i].str(), dst.simple_messages()[i].str());
    }

    EXPECT_EQ(src.other_messages().size(), dst.other_messages().size());
    for (size_t i = 0; i < src.other_messages().size(); ++i) {
        EXPECT_EQ(
                src.other_messages()[i].other_num(),
                dst.other_messages()[i].other_num());
    }
}

TEST(EncodeDecodeTest, NumericTypesTest) {
    stringstream ss;
    StlInputStream iss(&ss);
    StlOutputStream oss(&ss);

    NumericTypes src;
    src.set_int32_value(123);
    src.set_int64_value(1234567890);
    src.set_uint32_value(456);
    src.set_uint64_value(9876543210);
    src.set_sint32_value(-123);
    src.set_sint64_value(-1234567890);
    src.set_fixed32_value(123);
    src.set_fixed64_value(1234567890);
    src.set_sfixed32_value(-123);
    src.set_sfixed64_value(-1234567890);
    src.set_float_value(3.14f);
    src.set_double_value(2.71828);

    size_t size;
    // Encode the message to the stream.
    EXPECT_TRUE(EncodeMessage(oss, src, size));

    // Decode the message from the stream.
    NumericTypes dst;
    EXPECT_TRUE(DecodeMessage(iss, &dst));

    // Let's compare the fields of the source and destination messages.
    EXPECT_EQ(src.int32_value(), dst.int32_value());
    EXPECT_EQ(src.int64_value(), dst.int64_value());
    EXPECT_EQ(src.uint32_value(), dst.uint32_value());
    EXPECT_EQ(src.uint64_value(), dst.uint64_value());
    EXPECT_EQ(src.sint32_value(), dst.sint32_value());
    EXPECT_EQ(src.sint64_value(), dst.sint64_value());
    EXPECT_EQ(src.fixed32_value(), dst.fixed32_value());
    EXPECT_EQ(src.fixed64_value(), dst.fixed64_value());
    EXPECT_EQ(src.sfixed32_value(), dst.sfixed32_value());
    EXPECT_EQ(src.sfixed64_value(), dst.sfixed64_value());
    EXPECT_NEAR(src.float_value(), dst.float_value(), EPSILON);
    EXPECT_NEAR(src.double_value(), dst.double_value(), EPSILON);
}

TEST(EncodeDecodeTest, RepeatedNumericTypesTest) {
    stringstream ss;
    StlInputStream iss(&ss);
    StlOutputStream oss(&ss);

    RepeatedNumericTypes src;
    *src.add_int32_values() = 123;
    *src.add_int32_values() = 0;
    *src.add_int32_values() = 1230;
    *src.add_int32_values() = -1230;

    *src.add_int64_values() = 1234567890;
    *src.add_int64_values() = 0;
    *src.add_int64_values() = -1234567890;

    *src.add_uint32_values() = 456;
    *src.add_uint32_values() = 0;
    *src.add_uint32_values() = 4560;

    *src.add_uint64_values() = 9876543210;
    *src.add_uint64_values() = 0;
    *src.add_uint64_values() = 98765;

    *src.add_sint32_values() = -123;
    *src.add_sint32_values() = 0;
    *src.add_sint32_values() = 123;

    *src.add_sint64_values() = -1234567890;
    *src.add_sint64_values() = 0;
    *src.add_sint64_values() = 1234567890;

    *src.add_fixed32_values() = 123;
    *src.add_fixed32_values() = 0;
    *src.add_fixed32_values() = 321;

    *src.add_fixed64_values() = 1234567890;
    *src.add_fixed64_values() = 0;
    *src.add_fixed64_values() = 123456;

    *src.add_sfixed32_values() = -123000;
    *src.add_sfixed32_values() = 0;
    *src.add_sfixed32_values() = 123000;

    *src.add_sfixed64_values() = -1234567890;
    *src.add_sfixed64_values() = 0;
    *src.add_sfixed64_values() = 1234567890;

    *src.add_float_values() = 3.14f;
    *src.add_float_values() = -3.14f;
    *src.add_float_values() = 0.0f;
    *src.add_float_values() = 0.08f;

    *src.add_double_values() = 2.71828;
    *src.add_double_values() = -2.71828;
    *src.add_double_values() = 0.0;
    *src.add_double_values() = 0.01;

    size_t size;
    // Encode the message to the stream.
    EXPECT_TRUE(EncodeMessage(oss, src, size));

    // Decode the message from the stream.
    RepeatedNumericTypes dst;
    EXPECT_TRUE(DecodeMessage(iss, &dst));

    // Let's compare the fields of the source and destination messages.
    testRepeatedField(src.int32_values(), dst.int32_values());
    testRepeatedField(src.int64_values(), dst.int64_values());
    testRepeatedField(src.uint32_values(), dst.uint32_values());
    testRepeatedField(src.uint64_values(), dst.uint64_values());
    testRepeatedField(src.sint32_values(), dst.sint32_values());
    testRepeatedField(src.sint64_values(), dst.sint64_values());
    testRepeatedField(src.fixed32_values(), dst.fixed32_values());
    testRepeatedField(src.fixed64_values(), dst.fixed64_values());
    testRepeatedField(src.sfixed32_values(), dst.sfixed32_values());
    testRepeatedField(src.sfixed64_values(), dst.sfixed64_values());
    testRepeatedField(src.float_values(), dst.float_values());
    testRepeatedField(src.double_values(), dst.double_values());
}
