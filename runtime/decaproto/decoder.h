#ifndef DECAPROTO_DECODER_H
#define DECAPROTO_DECODER_H

#include "decaproto/descriptor.h"
#include "decaproto/message.h"
#include "decaproto/stream/stream.h"

namespace decaproto {

inline WireType GetWireType(FieldType type) {
    switch (type) {
        case kInt32:
        case kInt64:
        case kUint32:
        case kUint64:
        case kSint32:
        case kSint64:
        case kBool:
        case kEnum:
            return kVarint;
        case kFixed32:
        case kSfixed32:
        case kFloat:
            return kI32;
        case kFixed64:
        case kSfixed64:
        case kDouble:
            return kI64;
        case kString:
        case kBytes:
        case kMessage:
            return kLen;
        default:
            return kVarint;
    }
};

bool DecodeMessage(InputStream& stream, Message* out);

}  // namespace decaproto

#endif  // DECAPROTO_DECODER_H
