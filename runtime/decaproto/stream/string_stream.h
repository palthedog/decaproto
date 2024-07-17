#ifndef STRING_STREAM_H
#define STRING_STREAM_H

#include "stream.h"
#include <string>

namespace decaproto {
    class StringOutputStream : public OutputStream {
    private:
        std::string* str_;
    public:
        StringOutputStream(std::string* str) : str_(str) {
        }

        virtual ~StringOutputStream() {
        }

        virtual bool Write(uint8_t ch) {
            str_->push_back(ch);
            return true;
        }
    };
}

#endif
