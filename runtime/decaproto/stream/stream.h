#ifndef DECAPROTO_STREAM_STREAM_H
#define DECAPROTO_STREAM_STREAM_H

#include <cstdint>

namespace decaproto {

class InputStream {
public:
    InputStream() {
    }
    virtual ~InputStream() {
    }

    virtual bool Read(std::uint8_t& out) = 0;
};

}  // namespace decaproto

#endif
