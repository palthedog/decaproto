#include "decaproto/stream/coded_stream.h"

namespace decaproto {

bool CodedInputStream::ReadVarint(uint64_t& result) {
    result = 0;
    uint32_t shift = 0;
    uint8_t b;
    while (input_->Read(b)) {
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

}  // namespace decaproto