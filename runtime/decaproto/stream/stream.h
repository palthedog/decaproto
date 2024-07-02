#ifndef DECAPROTO_STREAM_STREAM_H
#define DECAPROTO_STREAM_STREAM_H

#include <cstdint>

namespace decaproto {

// Let's keep the implementation as simple as possible for now.
class InputStream {
public:
    InputStream() {
    }
    virtual ~InputStream() {
    }

    virtual bool Read(std::uint8_t& out) = 0;
};

class OutputStream {
public:
    OutputStream() {
    }
    virtual ~OutputStream() {
    }

    virtual bool Write(std::uint8_t& out, uint8_t ch) = 0;
};

}  // namespace decaproto

#endif
