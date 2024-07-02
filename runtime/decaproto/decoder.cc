#include "decaproto/decoder.h"

#include "decaproto/message.h"
#include "decaproto/stream/coded_stream.h"
#include "decaproto/stream/stream.h"

// TODO: We need to remove it since Arduino doesn't support <iostream>.
//    Instead, we should privide a way to inject a logger.
#include <iostream>

// https://protobuf.dev/programming-guides/encoding/
/*
message    := (tag value)*

tag        := (field << 3) bit-or wire_type;
                encoded as uint32 varint
value      := varint      for wire_type == VARINT,
              i32         for wire_type == I32,
              i64         for wire_type == I64,
              len-prefix  for wire_type == LEN,
              <empty>     for wire_type == SGROUP or EGROUP

varint     := int32 | int64 | uint32 | uint64 | bool | enum | sint32 | sint64;
                encoded as varints (sintN are ZigZag-encoded first)
i32        := sfixed32 | fixed32 | float;
                encoded as 4-byte little-endian;
                memcpy of the equivalent C types (u?int32_t, float)
i64        := sfixed64 | fixed64 | double;
                encoded as 8-byte little-endian;
                memcpy of the equivalent C types (u?int64_t, double)

len-prefix := size (message | string | bytes | packed);
                size encoded as int32 varint
string     := valid UTF-8 string (e.g. ASCII);
                max 2GB of bytes
bytes      := any sequence of 8-bit bytes;
                max 2GB of bytes
packed     := varint* | i32* | i64*,
                consecutive values of the type specified in `.proto`
*/

using namespace std;

namespace decaproto {

bool DecodeTag(
        CodedInputStream& cis, uint32_t& field_number, WireType& wire_type) {
    // tag        := (field << 3) bit-or wire_type;
    //                 encoded as uint32 varint
    uint32_t tag;
    if (!cis.ReadVarint32(tag)) {
        std::cerr << "Failed to read tag" << std::endl;
        return false;
    }
    field_number = tag >> 3;
    wire_type = static_cast<WireType>(tag & 0x7);
    return true;
}

bool SkipUnknownField(CodedInputStream& cis, WireType wire_type) {
    switch (wire_type) {
        case kVarint: {
            uint64_t value;
            if (!cis.ReadVarint64(value)) {
                return false;
            }
            break;
        }
        case kI64: {
            uint64_t value;
            if (!cis.ReadFixedInt64(value)) {
                return false;
            }
            break;
        }
        case kLen: {
            uint32_t len;
            if (!cis.ReadVarint32(len)) {
                return false;
            }
            cis.Skip(len);
            break;
        }
        case kDeprecated_SGroup:
        case kDeprecated_EGroup:
        case kI32:
            return false;
    }
    return true;
}

bool DecodeVarint(
        CodedInputStream& cis,
        Message* message,
        const Reflection* reflection,
        const FieldDescriptor* field) {
    uint64_t value;
    if (!cis.ReadVarint64(value)) {
        return false;
    }

    uint32_t tag = field->GetTag();
    switch (field->GetType()) {
        case kInt32:
            reflection->SetInt32(message, tag, value);
            return true;
        case kUInt32:
            reflection->SetUInt32(message, tag, value);
            return true;
        case kBool:
            reflection->SetBool(message, tag, value);
            return true;
        case kEnum:
            reflection->SetEnumValue(message, tag, value);
            return true;
        case kInt64:
            reflection->SetInt64(message, tag, value);
            return true;
        case kUInt64:
            reflection->SetUInt64(message, tag, value);
            return true;
        case kSInt32:
            // not supported yet
            // reflection->SetSInt32(message, tag,
            // CodedInputStream::DecodeZigzag(value));
            std::cerr << "SetSInt32 is not implemented yet" << std::endl;
            return false;
        case kSInt64:
            // not supported yet
            // reflection->SetSInt64(message, tag, value);
            std::cerr << "SetSInt64 is not implemented yet" << std::endl;
            return false;
        default:
            // This field is not a varint field.
            std::cerr << "This field is not a varint field. tag: "
                      << field->GetTag() << std::endl;
            return false;
    }
    return true;
}

bool DecodeMessage(
        CodedInputStream& cis,
        Message* message,
        const Reflection* reflection,
        const Descriptor* descriptor) {
    //  message    := (tag value)*

    uint32_t field_number;
    WireType wire_type;
    while (DecodeTag(cis, field_number, wire_type)) {
        const FieldDescriptor* field =
                descriptor->FindFieldByNumber(field_number);
        if (field == nullptr) {
            // The field is not defined in the descriptor.
            // Skip the unknown field.

            // TODO: We should keep them as unknown fields so that we can encode
            // the message again without losing any information.
            SkipUnknownField(cis, wire_type);
            continue;
        }
        if (GetWireType(field->GetType()) != wire_type) {
            // The wire type does not match the field type.
            // It happens because the sender and the receiver have different
            // proto definitions.
            std::cerr << "The wire type doesn't match the field type."
                      << " Field type: " << field->GetType()
                      << ", WireType(FieldType)"
                      << GetWireType(field->GetType())
                      << ", Wire type: " << wire_type << std::endl;
            return false;
        }

        if (field->IsRepeated()) {
            // TODO: Implement it
            std::cerr << "Decoding repeated fields is not implemented yet."
                      << endl;
            return false;
        }

        switch (wire_type) {
            case kVarint:
                if (!DecodeVarint(cis, message, reflection, field)) {
                    return false;
                }
                break;
            case kI64:
            case kI32:
                std::cerr << "fixed int is not supported yet." << endl;
                return false;
            case kLen:
            case kDeprecated_SGroup:
            case kDeprecated_EGroup:
                // TODO: Implement it
                std::cerr << "klen" << endl;
                return false;
        }
    }
    return true;
}

bool DecodeMessage(InputStream& ins, Message* out) {
    CodedInputStream cis(&ins);
    return DecodeMessage(cis, out, out->GetReflection(), out->GetDescriptor());
}

}  // namespace decaproto