#ifndef DECAPROTO_CODED_STREAM_H
#define DECAPROTO_CODED_STREAM_H

#include <string>

#include "decaproto/stream/stream.h"

namespace decaproto {

// Wraps InputStream to provide additional functionality.
class StreamWrapper final {
    InputStream* input_;
    size_t consumed_;

public:
    StreamWrapper(InputStream* input) : input_(input), consumed_(0) {
    }

    bool Read(std::uint8_t& out) {
        if (!input_->Read(out)) {
            return false;
        }
        consumed_++;
        return true;
    }

    // How much data has been consumed from the stream.
    size_t ConsumedSize() {
        return consumed_;
    };
};

// Reads and decodes a varint from the input stream.
class CodedInputStream {
public:
    CodedInputStream(InputStream* input) : input_(input) {
    }

    ~CodedInputStream() {
    }

    void Skip(int len) {
        // Temporal implementation to InputStream's interface as simple as
        // possible while developing the library.
        uint8_t b;
        for (int i = 0; i < len; i++) {
            input_.Read(b);
        }
    }

    size_t ConsumedSize() {
        return input_.ConsumedSize();
    }

    bool ReadString(std::string& result, int len) {
        result.clear();
        for (int i = 0; i < len; i++) {
            uint8_t b;
            if (!input_.Read(b)) {
                return false;
            }
            result.push_back(b);
        }
        return true;
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
        uint64_t uint64_result;
        if (!ReadVarint64(uint64_result)) {
            return false;
        }
        result = DecodeZigZag(uint64_result);
        return true;
    }

    bool ReadSignedVarint32(int32_t& result) {
        int64_t result64;
        if (!ReadSignedVarint64(result64)) {
            return false;
        }
        result = static_cast<int32_t>(result64);
        return result64 >= INT32_MIN && result64 <= INT32_MAX;
    }

    bool ReadFixedInt32(uint32_t& result);
    bool ReadFixedInt64(uint64_t& result);

    static int64_t DecodeZigZag(uint64_t value) {
        return (value >> 1) ^ -(value & 1);
    }

private:
    StreamWrapper input_;
};

}  // namespace decaproto

#endif