#include "decaproto/stream/coded_stream.h"

namespace decaproto {

bool CodedInputStream::ReadVarint(uint64_t& result) {
    result = 0;
    int shift = 0;
    uint8_t b;
    while (input_->Read(b)) {
        result |= static_cast<uint64_t>(b & 0x7f) << shift;
        if ((b & 0x80) == 0) {
            return true;
        }
        shift += 7;

        if (shift >= 64) {
            // overflow
            return false;
        }
    }

    // End of stream arrived before the end of varint
    return false;
}

bool CodedInputStream::ReadSignedVarint(int64_t& result) {
    uint64_t zig_zag_enced;
    if (!ReadVarint(zig_zag_enced)) {
        return false;
    }
    result = (zig_zag_enced >> 1) ^ -(zig_zag_enced & 1);
    return true;
}

}  // namespace decaproto