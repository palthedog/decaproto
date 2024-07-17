#ifndef DECAPROTO_MESSAGE_H
#define DECAPROTO_MESSAGE_H

#include <iostream>

#include "decaproto/descriptor.h"
#include "decaproto/reflection.h"
#include "decaproto/stream/coded_stream.h"
#include "decaproto/stream/stream.h"

namespace decaproto {

// Base class for all messages.
class Message {
public:
    Message() {
    }

    virtual ~Message() {
    }

    bool Encode(OutputStream& stream, size_t& written_size) const {
        CodedOutputStream cos(&stream);
        bool result = this->EncodeImpl(cos);
        written_size = cos.WrittenSize();
        return result;
    }

    virtual bool EncodeImpl(CodedOutputStream& stream) const = 0;
    virtual size_t ComputeEncodedSize() const = 0;

    virtual const Descriptor* GetDescriptor() const = 0;
    virtual const Reflection* GetReflection() const = 0;
};

}  // namespace decaproto

#endif
