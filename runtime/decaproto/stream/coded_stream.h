#ifndef DECAPROTO_CODED_STREAM_H
#define DECAPROTO_CODED_STREAM_H

#include "decaproto/stream/stream.h"

namespace decaproto {

// https://protobuf.dev/programming-guides/encoding/
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