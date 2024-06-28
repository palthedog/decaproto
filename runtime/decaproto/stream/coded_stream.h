#ifndef DECAPROTO_CODED_STREAM_H
#define DECAPROTO_CODED_STREAM_H

#include "decaproto/stream/stream.h"

namespace decaproto {

// https://protobuf.dev/programming-guides/encoding/
/*
message    := (tag value)*

tag        := (field << 3) bit-or wire_type;
                encoded as uint32 varint
value      := varint      for wire_type == VARINT,
              i32         for wire_type == I32,
              i64         for wire_type == I64,
              len-prefix  for wire_type == LEN,
              <empty>     for wire_type == SGROUP or EGROUP

varint     := int32 | int64 | uint32 | uint64 | bool | enum | sint32 | sint64;
                encoded as varints (sintN are ZigZag-encoded first)
i32        := sfixed32 | fixed32 | float;
                encoded as 4-byte little-endian;
                memcpy of the equivalent C types (u?int32_t, float)
i64        := sfixed64 | fixed64 | double;
                encoded as 8-byte little-endian;
                memcpy of the equivalent C types (u?int64_t, double)

len-prefix := size (message | string | bytes | packed);
                size encoded as int32 varint
string     := valid UTF-8 string (e.g. ASCII);
                max 2GB of bytes
bytes      := any sequence of 8-bit bytes;
                max 2GB of bytes
packed     := varint* | i32* | i64*,
                consecutive values of the type specified in `.proto`
*/

class CodedInputStream {
public:
    CodedInputStream(InputStream* input) : input_(input) {
    }

    ~CodedInputStream() {
    }

    bool ReadVarint64(uint64_t& result);

    bool ReadVarint32(uint32_t& result) {
        uint64_t result64;
        if (!ReadVarint64(result64)) {
            return false;
        }
        result = static_cast<uint32_t>(result64);
        return result64 <= UINT32_MAX;
    }

    bool ReadSignedVarint64(int64_t& result) {
        return ReadSignedVarint64Impl(result);
    }

    bool ReadSignedVarint32(int32_t& result) {
        int64_t result64;
        if (!ReadSignedVarint64Impl(result64)) {
            return false;
        }
        result = static_cast<int32_t>(result64);
        return result64 >= INT32_MIN && result64 <= INT32_MAX;
    }

    bool ReadFixedInt32(uint32_t& result);
    bool ReadFixedInt64(uint64_t& result);

private:
    InputStream* input_;

    bool ReadSignedVarint64Impl(int64_t& result) {
        uint64_t zig_zag_enced;
        if (!ReadVarint64(zig_zag_enced)) {
            return false;
        }
        result = (zig_zag_enced >> 1) ^ -(zig_zag_enced & 1);
        return true;
    }
};

}  // namespace decaproto

#endif