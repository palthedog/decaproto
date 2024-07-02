#ifndef DECAPROTO_STL_H
#define DECAPROTO_STL_H

#include <iostream>

#include "decaproto/stream/stream.h"

namespace decaproto {

class StlInputStream : public InputStream {
    std::istream* stream_;

public:
    StlInputStream(std::istream* stream) : stream_(stream) {
    }

    bool Read(std::uint8_t& out) override {
        char c;
        stream_->get(c);
        out = static_cast<std::uint8_t>(c);
        return static_cast<bool>(*stream_);
    }
};

class StlOutputStream : public OutputStream {
    std::ostream* stream_;

public:
    StlOutputStream(std::ostream* stream) : stream_(stream) {
    }

    bool Write(uint8_t ch) override {
        stream_->put(ch);
        return stream_;
    }
};

}  // namespace decaproto

#endif
