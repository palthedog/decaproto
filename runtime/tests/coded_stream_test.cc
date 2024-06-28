#include "decaproto/stream/coded_stream.h"

#include <gtest/gtest.h>

#include <sstream>

#include "decaproto/stream/stl_stream_wrapper.h"

using namespace decaproto;
using namespace std;

// https://protobuf.dev/programming-guides/encoding/

TEST(StreamTest, ReadOneTest) {
    stringstream ss;
    // 1 is encoded as 0000 0001
    ss.put(0x01);

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;
    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(1, result);
}

TEST(StreamTest, ReadTest) {
    stringstream ss;
    // 150 is encoded as 1001_0110 0000_0001
    ss.put(0x96);
    ss.put(0x01);

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;
    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(150, result);
}

TEST(StreamTest, ReadUint32Test) {
    stringstream ss;
    // 150 is encoded as 1001_0110 0000_0001
    ss.put(0x96);
    ss.put(0x01);

    CodedInputStream cis(new StlInputStream(&ss));
    uint32_t result;
    EXPECT_TRUE(cis.ReadVarint32(result));
    EXPECT_EQ(150, result);
}

TEST(StreamTest, SignedTest) {
    stringstream ss;
    // Signed Value: Encoded as ZigZag
    // 0: 0
    // -1: 1
    // 1: 2
    // -2: 3
    ss.put(0x00);
    ss.put(0x01);
    ss.put(0x02);
    ss.put(0x03);

    CodedInputStream cis(new StlInputStream(&ss));
    int64_t result;
    EXPECT_TRUE(cis.ReadSignedVarint64(result));
    EXPECT_EQ(0, result);

    EXPECT_TRUE(cis.ReadSignedVarint64(result));
    EXPECT_EQ(-1, result);

    EXPECT_TRUE(cis.ReadSignedVarint64(result));
    EXPECT_EQ(1, result);

    EXPECT_TRUE(cis.ReadSignedVarint64(result));
    EXPECT_EQ(-2, result);
}

TEST(StreamTest, SignedInt32Test) {
    stringstream ss;
    // Signed Value: Encoded as ZigZag
    // 0: 0
    // -1: 1
    // 1: 2
    // -2: 3
    ss.put(0x00);
    ss.put(0x01);
    ss.put(0x02);
    ss.put(0x03);

    CodedInputStream cis(new StlInputStream(&ss));
    int32_t result;
    EXPECT_TRUE(cis.ReadSignedVarint32(result));
    EXPECT_EQ(0, result);

    EXPECT_TRUE(cis.ReadSignedVarint32(result));
    EXPECT_EQ(-1, result);

    EXPECT_TRUE(cis.ReadSignedVarint32(result));
    EXPECT_EQ(1, result);

    EXPECT_TRUE(cis.ReadSignedVarint32(result));
    EXPECT_EQ(-2, result);
}

TEST(StreamTest, BiggerSignedTest) {
    stringstream ss;
    // Signed Value: Encoded as ZigZag
    // Writing maximum 64-bit value: 0xFFFFFFFFFFFFFFFF
    // Varint encoding requires continuation bits, so the encoded form is longer
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);  // 7bits * 9 = 63 bits
    ss.put(0x01);  // Now it's 0xFFFF_FFFF_FFFF_FFFF

    // Put same value again
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0x01);

    CodedInputStream cis(new StlInputStream(&ss));

    // Parse first value as unsigned value
    uint64_t unsigned_value;
    EXPECT_TRUE(cis.ReadVarint64(unsigned_value));
    EXPECT_EQ(UINT64_MAX, unsigned_value);

    // Parse first value as signed value
    int64_t signed_value;
    EXPECT_TRUE(cis.ReadSignedVarint64(signed_value));
    EXPECT_EQ(INT64_MIN, signed_value);
}

TEST(StreamTest, ReadBigValueTest) {
    stringstream ss;
    // A number bigger than 32 bits max
    // 0x80 0x80 0x80 0x80 0x10
    // 7 + 7 + 7 + 7 + 4 = 32 bits
    ss.put(0x80);
    ss.put(0x80);
    ss.put(0x80);
    ss.put(0x80);
    ss.put(0x10);

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;
    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(uint64_t{1} << 32, result);
}

TEST(StreamTest, ReadOverflow32bitsTest) {
    stringstream ss;
    // A number bigger than 32 bits max
    // 0x80 0x80 0x80 0x80 0x10
    // 7 + 7 + 7 + 7 + 4 = 32 bits
    ss.put(0x80);
    ss.put(0x80);
    ss.put(0x80);
    ss.put(0x80);
    ss.put(0x10);

    CodedInputStream cis(new StlInputStream(&ss));
    uint32_t result;
    EXPECT_FALSE(cis.ReadVarint32(result));
}

TEST(StreamTest, ReadMultipleVarintsTest) {
    stringstream ss;
    // Writing multiple varints to stream: 10, 200, 30000
    // 10
    ss.put(0x0A);

    // 150
    ss.put(0x96);
    ss.put(0x01);

    // 0x03_02_01 in two bytes
    ss.put(0x80 | (0x01 << 0));
    ss.put(0x80 | (0x02 << 1));
    ss.put(0x00 | (0x03 << 2));

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;

    // Test reading first varint
    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(10, result);

    // Test reading second varint
    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(150, result);

    // Test reading third varint
    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(0x030201, result);
}

TEST(StreamTest, ReadVarintWithMaxValueTest) {
    stringstream ss;
    // Writing maximum 64-bit value: 0xFFFFFFFFFFFFFFFF
    // Varint encoding requires continuation bits, so the encoded form is longer
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);  // 7bits * 9 = 63 bits
    ss.put(0x01);  // last one bit + MSO bit off to signal end of varint

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;

    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(UINT64_MAX, result);
}

TEST(StreamTest, ReadVarintWithZeroTest) {
    stringstream ss;
    // Writing a zero varint
    ss.put(0x00);  // Single byte for zero

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;

    EXPECT_TRUE(cis.ReadVarint64(result));
    EXPECT_EQ(0, result);
}

TEST(StreamTest, ReadVarintFailureTest) {
    stringstream ss;
    // Simulate a failure in reading varint by not putting any data in the
    // stream

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;
    EXPECT_FALSE(cis.ReadVarint64(result));

    // Try to read on an empty stream again
    EXPECT_FALSE(cis.ReadVarint64(result));
}

TEST(StreamTest, ReadVarintOverflowFailureTest) {
    stringstream ss;
    // Writing a varint that overflows 64 bits
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0xFF);
    ss.put(0x01);  // 7bits * 10 + 1 = 71 bits

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;
    EXPECT_FALSE(cis.ReadVarint64(result));
}

TEST(StreamTest, ReadFixedInt64Test) {
    stringstream ss;
    // Writing a varint that overflows 64 bits
    ss.put(0x01);
    ss.put(0x23);
    ss.put(0x45);
    ss.put(0x67);
    ss.put(0x89);
    ss.put(0xAB);
    ss.put(0xCD);
    ss.put(0xEF);

    CodedInputStream cis(new StlInputStream(&ss));
    uint64_t result;
    EXPECT_TRUE(cis.ReadFixedInt64(result));
    EXPECT_EQ(0xEFCDAB8967452301, result);
}

TEST(StreamTest, ReadFixedInt32Test) {
    stringstream ss;
    // Writing a varint that overflows 64 bits
    ss.put(0x01);
    ss.put(0x23);
    ss.put(0x45);
    ss.put(0x67);

    CodedInputStream cis(new StlInputStream(&ss));
    uint32_t result;
    EXPECT_TRUE(cis.ReadFixedInt32(result));
    EXPECT_EQ(0x67452301, result);
}
