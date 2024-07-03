#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/decoder.h"
#include "decaproto/encoder.h"
#include "decaproto/stream/stl.h"
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
    mut_simple_messages->resize(2);
    (*mut_simple_messages)[0].set_str("foo");
    (*mut_simple_messages)[1].set_str("bar");

    std::vector<OtherMessage>* mut_other_messages =
            src.mutable_other_messages();
    mut_other_messages->resize(2);
    (*mut_other_messages)[0].set_other_num(100);
    (*mut_other_messages)[1].set_other_num(200);

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
