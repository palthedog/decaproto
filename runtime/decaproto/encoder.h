#ifndef DECAPROTO_ENCODER_H
#define DECAPROTO_ENCODER_H

#include "decaproto/message.h"
#include "decaproto/stream/stream.h"

namespace decaproto {

size_t ComputeEncodedSize(const Message& message);

bool EncodeMessage(
        OutputStream& stream, const Message& message, size_t& written_size);

}  // namespace decaproto

#endif  // DECAPROTO_ENCODER_H
