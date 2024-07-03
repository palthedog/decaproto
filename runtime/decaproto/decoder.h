#ifndef DECAPROTO_DECODER_H
#define DECAPROTO_DECODER_H

#include "decaproto/descriptor.h"
#include "decaproto/message.h"
#include "decaproto/stream/stream.h"

namespace decaproto {

enum WireType {
    // 0	VARINT	int32, int64, uint32, uint64, sint32, sint64, bool, enum
    kVarint = 0,
    // 1	I64	fixed64, sfixed64, double
    kI64 = 1,
    // 2	LEN	string, bytes, embedded messages, packed repeated fields
    kLen = 2,
    // 3	SGROUP	group start (deprecated)
    kDeprecated_SGroup = 3,
    // 4	EGROUP	group end (deprecated)
    kDeprecated_EGroup = 4,
    // 5	I32	fixed32, sfixed32, float
    kI32 = 5,
};

inline WireType GetWireType(FieldType type) {
    switch (type) {
        case kInt32:
        case kInt64:
        case kUInt32:
        case kUInt64:
        case kSInt32:
        case kSInt64:
        case kBool:
        case kEnum:
            return kVarint;
        case kFixed32:
        case kSFixed32:
        case kFloat:
            return kI32;
        case kFixed64:
        case kSFixed64:
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
