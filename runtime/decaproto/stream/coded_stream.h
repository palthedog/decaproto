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

    bool ReadVarint(uint64_t& result);
    bool ReadSignedVarint(int64_t& result);

private:
    InputStream* input_;
};

}  // namespace decaproto

#endif