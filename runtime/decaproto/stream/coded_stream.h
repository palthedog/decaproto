#ifndef DECAPROTO_CODED_STREAM_H
#define DECAPROTO_CODED_STREAM_H

#include <string>

#include "decaproto/stream/stream.h"

namespace decaproto {

// Wraps InputStream to provide additional functionality.
class InputStreamWrapper final {
    InputStream* input_;
    size_t consumed_;

public:
    InputStreamWrapper(InputStream* input) : input_(input), consumed_(0) {
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

// Wraps OutputStream to provide additional functionality.
class OutputStreamWrapper final {
    OutputStream* output_;
    size_t written_;

public:
    OutputStreamWrapper(OutputStream* output) : output_(output), written_(0) {
    }

    bool Write(std::uint8_t value) {
        if (!output_->Write(value)) {
            return false;
        }
        written_++;
        return true;
    }

    // How much data has been written to the stream.
    size_t WrittenSize() {
        return written_;
    };
};

// Reads and decodes a varint from the input stream.
class CodedInputStream {
public:
    CodedInputStream(InputStream* input) : input_(input) {
    }

    ~CodedInputStream() {
    }

    void Skip(size_t len) {
        // Temporal implementation to InputStream's interface as simple as
        // possible while developing the library.
        uint8_t b;
        for (size_t i = 0; i < len; i++) {
            input_.Read(b);
        }
    }

    size_t ConsumedSize() {
        return input_.ConsumedSize();
    }

    bool ReadString(std::string& result, size_t len) {
        result.clear();
        for (size_t i = 0; i < len; i++) {
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
        uint64_t uresult;
        if (!ReadVarint64(uresult)) {
            return false;
        }
        result = DecodeZigZag64(uresult);
        return true;
    }

    bool ReadSignedVarint32(int32_t& result) {
        uint32_t uresult;
        if (!ReadVarint32(uresult)) {
            return false;
        }
        result = DecodeZigZag32(uresult);
        return true;
    }

    bool ReadFixedInt32(uint32_t& result);
    bool ReadFixedInt64(uint64_t& result);

    static int64_t DecodeZigZag64(uint64_t value) {
        return (value >> 1) ^ -(value & 1);
    }

    static int32_t DecodeZigZag32(uint32_t value) {
        return (value >> 1) ^ -(value & 1);
    }

private:
    InputStreamWrapper input_;
};

class CodedOutputStream {
public:
    CodedOutputStream(OutputStream* output) : output_(output) {
    }

    ~CodedOutputStream() {
    }

    size_t WrittenSize() {
        return output_.WrittenSize();
    }

    bool WriteString(const std::string& result) {
        for (size_t i = 0; i < result.size(); i++) {
            uint8_t b = result[i];
            if (!output_.Write(b)) {
                return false;
            }
        }
        return true;
    }

    bool WriteVarint64(uint64_t value);

    bool WriteVarint32(uint32_t value) {
        return WriteVarint64(value);
    }

    bool WriteSignedVarint64(int64_t value) {
        uint64_t zigzag_enc_value = EncodeZigZag(value);
        return WriteVarint64(zigzag_enc_value);
    }

    bool WriteSignedVarint32(int32_t value) {
        return WriteSignedVarint64(value);
    }

    bool WriteFixedInt32(uint32_t value);
    bool WriteFixedInt64(uint64_t value);

    static uint64_t EncodeZigZag(int64_t value) {
        return (value << 1) ^ (value >> 63);
    }

    static uint32_t EncodeZigZag32(int32_t value) {
        return (value << 1) ^ (value >> 31);
    }

private:
    OutputStreamWrapper output_;
};

}  // namespace decaproto

#endif  // DECAPROTO_CODED_STREAM_H
