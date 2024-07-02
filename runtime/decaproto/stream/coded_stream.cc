#include "decaproto/stream/coded_stream.h"

namespace decaproto {

bool CodedInputStream::ReadVarint64(uint64_t& result) {
    result = 0;
    uint32_t shift = 0;
    uint8_t b;
    while (input_.Read(b)) {
        result |= static_cast<uint64_t>(b & 0x7f) << shift;
        shift += 7;
        if ((b & 0x80) == 0) {
            return true;
        }

        if (shift >= 64) {
            // overflow
            return false;
        }
    }

    // End of stream arrived before the end of varint
    return false;
}

bool CodedInputStream::ReadFixedInt32(uint32_t& result) {
    result = 0;
    uint8_t b;
    for (int i = 0; i < 4; i++) {
        if (!input_.Read(b)) {
            return false;
        }
        // Little-endian
        result |= static_cast<uint32_t>(b) << (i * 8);
    }
    return true;
}

bool CodedInputStream::ReadFixedInt64(uint64_t& result) {
    result = 0;
    uint8_t b;
    for (int i = 0; i < 8; i++) {
        if (!input_.Read(b)) {
            return false;
        }
        // Little-endian
        result |= static_cast<uint64_t>(b) << (i * 8);
    }
    return true;
}

}  // namespace decaproto