#include "decaproto/decoder.h"

#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/stream/coded_stream.h"
#include "decaproto/stream/stl_stream_wrapper.h"
#include "decaproto/stream/stream.h"
#include "fake_message.h"

using namespace decaproto;
using namespace std;

TEST(DecoderTest, DecodeSingleNumTest) {
    stringstream ss;
    // 08 96 01
    // tag: 08 (00001000)
    //   -> 0(continuation bit)
    //      0001(field_number)
    //      000(wire_type)
    ss.put(0x08);
    // value: 96 01
    //   -> varint 150
    ss.put(0x96);
    ss.put(0x01);

    StlInputStream ins(&ss);

    FakeMessage m;
    EXPECT_EQ(
            FieldType::kUInt32,
            m.GetDescriptor()->FindFieldByNumber(kNumTag)->GetType());

    EXPECT_TRUE(DecodeMessage(ins, &m));
}
