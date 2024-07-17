#include "decaproto/encoder.h"

#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/stream/coded_stream.h"
#include "decaproto/stream/stl.h"
#include "decaproto/stream/stream.h"
#include "fake_message.h"

using namespace decaproto;
using namespace std;

TEST(EncoderTest, VarintSizeTest) {
    FakeMessage m;
    m.set_num(150);

    // tag(1) + varint(2)
    EXPECT_EQ(3, m.ComputeEncodedSize());
}

TEST(EncoderTest, DefaultVarintSizeTest) {
    FakeMessage m;
    m.set_num(0);

    EXPECT_EQ(0, m.ComputeEncodedSize());
}

TEST(EncoderTest, StringSizeTest) {
    FakeMessage m;
    m.set_str("testing");

    // tag(1) + len(1) + value(7)
    EXPECT_EQ(9, m.ComputeEncodedSize());
}

TEST(EncoderTest, DefaultStringSizeTest) {
    FakeMessage m;
    m.set_str("");

    EXPECT_EQ(0, m.ComputeEncodedSize());
}

TEST(EncoderTest, EnumValueSizeTest) {
    FakeMessage m;
    m.set_enum_field(FakeEnum::ENUM_B);

    // tag(1) + varint(1)
    EXPECT_EQ(2, m.ComputeEncodedSize());
}

TEST(EncoderTest, RepeatedNumSizeTest) {
    FakeMessage m;
    m.mutable_rep_nums()->push_back(10);
    m.mutable_rep_nums()->push_back(150);
    m.mutable_rep_nums()->push_back(20);

    // tag(1) + varint(1) + tag(1) + varint(2) + tag(1) + varint(1)
    EXPECT_EQ(7, m.ComputeEncodedSize());
}

TEST(EncoderTest, RepeatedNumWithDefaultValueSizeTest) {
    FakeMessage m;
    m.mutable_rep_nums()->push_back(10);
    // Default value.
    // Since this is a repeated field, the default value should be encoded as
    // well
    m.mutable_rep_nums()->push_back(0);
    m.mutable_rep_nums()->push_back(20);

    // tag(1) + varint(1) + tag(1) + varint(1) + tag(1) + varint(1)
    EXPECT_EQ(6, m.ComputeEncodedSize());
}

TEST(EncoderTest, SubMessageSizeTest) {
    FakeMessage m;
    m.mutable_other()->set_num(150);

    // tag(1) + len(1)
    // sub_message(3)
    EXPECT_EQ(5, m.ComputeEncodedSize());
}

TEST(EncoderTest, EncodeNumTest) {
    stringstream ss;
    StlOutputStream outs(&ss);

    FakeMessage m;
    // 150 is encoded as 0x96 0x01 in varint
    m.set_num(150);

    size_t written_size;
    EXPECT_TRUE(m.Encode(outs, written_size));
    EXPECT_EQ(3, written_size);

    EXPECT_EQ(0x08, ss.get());
    EXPECT_EQ(0x96, ss.get());
    EXPECT_EQ(0x01, ss.get());
}

TEST(EncoderTest, EncodeStringTest) {
    stringstream ss;
    StlOutputStream outs(&ss);

    FakeMessage m;
    m.set_str("testing");

    size_t written_size;
    EXPECT_TRUE(m.Encode(outs, written_size));
    EXPECT_EQ(9, written_size);

    EXPECT_EQ(0x12, ss.get());
    EXPECT_EQ(0x07, ss.get());
    EXPECT_EQ('t', ss.get());
    EXPECT_EQ('e', ss.get());
    EXPECT_EQ('s', ss.get());
    EXPECT_EQ('t', ss.get());
    EXPECT_EQ('i', ss.get());
    EXPECT_EQ('n', ss.get());
    EXPECT_EQ('g', ss.get());
}

TEST(EncoderTest, EncodeSubMessageTest) {
    stringstream ss;
    StlOutputStream outs(&ss);

    FakeMessage m;
    m.mutable_other()->set_num(150);

    size_t written_size;
    EXPECT_TRUE(m.Encode(outs, written_size));
    EXPECT_EQ(5, written_size);

    EXPECT_EQ(0x1A, ss.get());
    EXPECT_EQ(0x03, ss.get());
    EXPECT_EQ(0x08, ss.get());
    EXPECT_EQ(0x96, ss.get());
    EXPECT_EQ(0x01, ss.get());
}
