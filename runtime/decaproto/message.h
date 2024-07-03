#ifndef DECAPROTO_MESSAGE_H
#define DECAPROTO_MESSAGE_H

#include <iostream>

#include "decaproto/descriptor.h"
#include "decaproto/reflection.h"

namespace decaproto {

// Base class for all messages.
class Message {
public:
    Message() {
    }

    virtual ~Message() {
        std::cerr << "destracting Message" << std::endl;
    }

    virtual const Descriptor* GetDescriptor() const = 0;
    virtual const Reflection* GetReflection() const = 0;
};

}  // namespace decaproto

#endif
