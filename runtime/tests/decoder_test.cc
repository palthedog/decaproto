#include "decaproto/decoder.h"

#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/stream/coded_stream.h"
#include "decaproto/stream/stl_stream_wrapper.h"
#include "decaproto/stream/stream.h"
#include "fake_message.h"

using namespace decaproto;
using namespace std;

TEST(DecoderTest, DecodeSingularNumTest) {
    stringstream ss;
    // 08 96 01
    // tag: 08 (00001000)
    //   -> 0(continuation bit: stop)
    //      0001(field_number: 1)
    //      000(wire_type: varint)
    ss.put(0b0'0001'000);
    // value: 96 01
    //   -> varint 150
    ss.put(0x96);
    ss.put(0x01);

    StlInputStream ins(&ss);

    FakeMessage m;
    // Check that FakeMessage's field-1 is kUInt32 just to make sure
    EXPECT_EQ(
            FieldType::kUInt32,
            m.GetDescriptor()->FindFieldByNumber(kNumTag)->GetType());

    EXPECT_TRUE(DecodeMessage(ins, &m));
    EXPECT_EQ(150, m.num());
}

TEST(DecoderTest, DecodeSingularStringTest) {
    stringstream ss;
    // 12 07 [74 65 73 74 69 6e 67]
    // tag: 12 (0 0010 010)
    //   -> 0(continuation bit: stop)
    //      0010(field_number: 2)
    //      010(wire_type: 2, len)
    ss.put(0b0'0010'010);
    // len: 07
    ss.put(0x07);
    // value: [74 65 73 74 69 6e 67]
    //   -> "testing"
    ss.put(0x74);
    ss.put(0x65);
    ss.put(0x73);
    ss.put(0x74);
    ss.put(0x69);
    ss.put(0x6e);
    ss.put(0x67);

    StlInputStream ins(&ss);

    FakeMessage m;
    // Check that FakeMessage's field-2 is kUInt32 just to make sure
    EXPECT_EQ(
            FieldType::kString,
            m.GetDescriptor()->FindFieldByNumber(kStrTag)->GetType());

    EXPECT_TRUE(DecodeMessage(ins, &m));
    EXPECT_EQ("testing", m.str());
}

TEST(DecoderTest, DecodeMessageFieldTest) {
    stringstream ss;
    // 1A 03 [08 96 01]
    // tag: 1A (0 0011 010)
    //   -> 0(continuation bit: stop)
    //      0011(field_number: 3)
    //      010(wire_type: 2, len)
    ss.put(0b0'0011'010);
    // len: 03
    ss.put(0x03);
    // value: [08 96 01]
    //   -> 1: 150
    ss.put(0x08);
    ss.put(0x96);
    ss.put(0x01);

    StlInputStream ins(&ss);

    FakeMessage m;
    // Check that FakeMessage's field-2 is kUInt32 just to make sure
    EXPECT_EQ(
            FieldType::kMessage,
            m.GetDescriptor()->FindFieldByNumber(kOtherTag)->GetType());

    EXPECT_TRUE(DecodeMessage(ins, &m));
    EXPECT_TRUE(m.has_other());
    EXPECT_EQ(150, m.other().num());
}
