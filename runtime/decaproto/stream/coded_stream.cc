#include "decaproto/stream/coded_stream.h"

#include <iostream>

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

bool CodedOutputStream::WriteVarint64(uint64_t value) {
    do {
        uint8_t b = value & 0x7f;
        value = value >> 7;
        if (value > 0) {
            // set continuation bit
            b |= 0x80;
        }
        if (!output_.Write(b)) {
            return false;
        }
    } while (value > 0);
    return true;
}

bool CodedOutputStream::WriteFixedInt32(uint32_t value) {
    for (int i = 0; i < 4; i++) {
        uint8_t b = value & 0xff;
        value = value >> 8;
        if (!output_.Write(b)) {
            return false;
        }
    }
    return true;
}

bool CodedOutputStream::WriteFixedInt64(uint64_t value) {
    for (int i = 0; i < 8; i++) {
        uint8_t b = value & 0xff;
        value = value >> 8;
        if (!output_.Write(b)) {
            return false;
        }
    }
    return true;
}

}  // namespace decaproto
