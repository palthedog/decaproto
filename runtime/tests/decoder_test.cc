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
    // Check that FakeMessage's field-1 is kUInt32 just for the confirmation
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
    // Check that FakeMessage's field-2 is kUInt32 just for the confirmation
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
    // Check that FakeMessage's field-2 is kUInt32 just for the confirmation
    EXPECT_EQ(
            FieldType::kMessage,
            m.GetDescriptor()->FindFieldByNumber(kOtherTag)->GetType());

    EXPECT_TRUE(DecodeMessage(ins, &m));
    EXPECT_TRUE(m.has_other());
    EXPECT_EQ(150, m.other().num());
}

TEST(DecoderTest, DecodeMessageFieldBetweenOtherFieldsTest) {
    stringstream ss;

    // 1: 10 (set num to 10)
    ss.put(0b0'0001'000);
    ss.put(0x0A);

    // 3: LEN 3 {1: 150}  (set other.num to 150)
    ss.put(0b0'0011'010);
    ss.put(0x03);
    ss.put(0x08);
    ss.put(0x96);
    ss.put(0x01);

    // 4: 2 (set enum_field to ENUM_B)
    ss.put(0b0'0100'000);
    ss.put(0x02);

    StlInputStream ins(&ss);

    FakeMessage m;
    // Check that FakeMessage's field-2 is kUInt32 just for the confirmation

    EXPECT_TRUE(DecodeMessage(ins, &m));

    // Checks sub message
    EXPECT_TRUE(m.has_other());
    EXPECT_EQ(150, m.other().num());

    // Checks outer message
    EXPECT_EQ(10, m.num());
    EXPECT_EQ(FakeEnum::ENUM_B, m.enum_field());
}

TEST(DecoderTest, DecodeEnumFieldTest) {
    stringstream ss;
    // 20 02 01
    // tag: 20 (0 0100 000)
    //   -> 0(continuation bit: stop)
    //      0100(field_number: 4)
    //      000(wire_type: varint)
    ss.put(0b0'0100'000);
    // value: 02
    //   -> 2 (ENUM_B)
    ss.put(0x02);

    StlInputStream ins(&ss);

    FakeMessage m;
    // Check that FakeMessage's field-2 is kUInt32 just for the confirmation
    EXPECT_EQ(
            FieldType::kEnum,
            m.GetDescriptor()->FindFieldByNumber(kEnumFieldTag)->GetType());

    EXPECT_TRUE(DecodeMessage(ins, &m));
    EXPECT_EQ(FakeEnum::ENUM_B, m.enum_field());
}

TEST(DecoderTest, DecodeRepeatedUInt32FieldTest) {
    stringstream ss;
    // tag: 2A (0 1010 010)
    //   -> 0(continuation bit: stop)
    //      0101(field_number: 5)
    //      000(wire_type: 0, varint)
    uint8_t tag = 0b0'0101'000;
    // values: 0A 96 01
    //   -> 10 150 20
    ss.put(tag);
    ss.put(0x0A);
    ss.put(tag);
    ss.put(0x96);
    ss.put(0x01);
    ss.put(tag);
    ss.put(0x14);

    StlInputStream ins(&ss);

    FakeMessage m;
    // Check that FakeMessage's field-2 is repeated kUInt32 just for the
    // confirmation
    EXPECT_EQ(
            FieldType::kUInt32,
            m.GetDescriptor()->FindFieldByNumber(kRepNumsTag)->GetType());
    EXPECT_TRUE(
            m.GetDescriptor()->FindFieldByNumber(kRepNumsTag)->IsRepeated());

    EXPECT_TRUE(DecodeMessage(ins, &m));
    EXPECT_EQ(3, m.rep_nums().size());
    EXPECT_EQ(10, m.rep_nums()[0]);
    EXPECT_EQ(150, m.rep_nums()[1]);
    EXPECT_EQ(20, m.rep_nums()[2]);
}

TEST(DecoderTest, DecodeRepeatedUInt32_StringInTheMiddle_FieldTest) {
    stringstream ss;
    // tag: 2A (0 1010 010)
    //   -> 0(continuation bit: stop)
    //      0101(field_number: 5)
    //      000(wire_type: 0, varint)
    uint8_t tag = 0b0'0101'000;
    // values: 0A 96 01
    //   -> 10 150 20

    // add nums
    ss.put(tag);
    ss.put(0x0A);

    // add nums
    ss.put(tag);
    ss.put(0x96);
    ss.put(0x01);

    // 2: LEN 7 {"testing"}
    ss.put(0b0'0010'010);
    ss.put(0x07);
    ss.put(0x74);
    ss.put(0x65);
    ss.put(0x73);
    ss.put(0x74);
    ss.put(0x69);
    ss.put(0x6e);
    ss.put(0x67);

    // add nums
    ss.put(tag);
    ss.put(0x14);

    StlInputStream ins(&ss);

    FakeMessage m;
    // Check that FakeMessage's field-2 is repeated kUInt32 just for the
    // confirmation
    EXPECT_EQ(
            FieldType::kUInt32,
            m.GetDescriptor()->FindFieldByNumber(kRepNumsTag)->GetType());
    EXPECT_TRUE(
            m.GetDescriptor()->FindFieldByNumber(kRepNumsTag)->IsRepeated());

    EXPECT_TRUE(DecodeMessage(ins, &m));
    EXPECT_EQ(3, m.rep_nums().size());
    EXPECT_EQ(10, m.rep_nums()[0]);
    EXPECT_EQ(150, m.rep_nums()[1]);
    EXPECT_EQ(20, m.rep_nums()[2]);

    EXPECT_EQ("testing", m.str());
}
