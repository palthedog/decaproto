#include "decaproto/encoder.h"

#include <iostream>
// For WireType
// It should be defined elsewhere?
#include "decaproto/decoder.h"
#include "decaproto/descriptor.h"
#include "decaproto/reflection.h"
#include "decaproto/stream/coded_stream.h"

using namespace std;

namespace decaproto {

namespace {

template <typename SRC_T, typename DST_T>
DST_T MemcpyCast(SRC_T src) {
    return *(DST_T*)(&src);
}

template <bool, typename DST_T>
DST_T MemcpyCast(bool src) {
    return src ? 1 : 0;
}

}  // namespace

bool EncodeMessage(
        CodedOutputStream& stream,
        const Message& message,
        const Reflection* reflection,
        const Descriptor* descriptor);

size_t ComputeEncodedVarintSize(uint64_t value) {
    size_t size = 0;
    do {
        value >>= 7;
        size++;
    } while (value != 0);
    return size;
}

size_t ComputeEncodedFieldSize(
        const Message& message,
        const Reflection* reflection,
        const FieldDescriptor& field_desc) {
    size_t size = 0;

    uint32_t tag = field_desc.GetFieldNumber();
    size_t field_count = 1;
    if (field_desc.IsRepeated()) {
        field_count = reflection->FieldSize(&message, tag);
    }

    switch (field_desc.GetType()) {
        case FieldType::kInt32:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto value = reflection->GetRepeatedInt32(&message, tag, i);
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                int32_t value = reflection->GetInt32(&message, tag);
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kSint32:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto value = CodedOutputStream::EncodeZigZag(
                            reflection->GetRepeatedSint32(&message, tag, i));
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                int32_t value = CodedOutputStream::EncodeZigZag(
                        reflection->GetSint32(&message, tag));
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kUint32:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto value =
                            reflection->GetRepeatedUint32(&message, tag, i);
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                uint32_t value = reflection->GetUint32(&message, tag);
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kBool:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto value =
                            reflection->GetRepeatedUint32(&message, tag, i);
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                uint32_t value = reflection->GetBool(&message, tag) ? 1 : 0;
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kInt64:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto value = reflection->GetRepeatedInt64(&message, tag, i);
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                int64_t value = reflection->GetInt64(&message, tag);
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kSint64:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    int64_t value = CodedOutputStream::EncodeZigZag(
                            reflection->GetRepeatedSint64(&message, tag, i));
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                int64_t value = CodedOutputStream::EncodeZigZag(
                        reflection->GetSint64(&message, tag));
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kUint64:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto value =
                            reflection->GetRepeatedUint64(&message, tag, i);
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                uint64_t value = reflection->GetUint64(&message, tag);
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kString:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto& value =
                            reflection->GetRepeatedString(&message, tag, i);
                    // tag
                    size += 1;
                    // LEN
                    size += ComputeEncodedVarintSize(value.size());
                    // value
                    size += value.size();
                }
            } else {
                // TODO: Implement hasser in reflection and use it to prevent
                // memory allocation here.
                string value = reflection->GetString(&message, tag);
                if (!value.empty()) {
                    // tag
                    size += 1;
                    // LEN
                    size += ComputeEncodedVarintSize(value.size());
                    // value
                    size += value.size();
                }
            }
            break;
        case FieldType::kEnum:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto value =
                            reflection->GetRepeatedEnumValue(&message, tag, i);
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                int32_t value = reflection->GetEnumValue(&message, tag);
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kMessage:
            if (field_desc.IsRepeated()) {
                for (size_t i = 0; i < field_count; i++) {
                    auto& value =
                            reflection->GetRepeatedMessage(&message, tag, i);
                    size_t sub_msg_size = ComputeEncodedSize(value);
                    // tag
                    size += 1;
                    // LEN
                    size += ComputeEncodedVarintSize(sub_msg_size);
                    // value
                    size += sub_msg_size;
                }
            } else {
                const Message& value = reflection->GetMessage(&message, tag);
                size_t sub_msg_size = ComputeEncodedSize(value);
                if (sub_msg_size > 0) {
                    // Encode only if the sub message is non-default value
                    // tag
                    size += 1;
                    // LEN
                    size += ComputeEncodedVarintSize(sub_msg_size);
                    // value
                    size += sub_msg_size;
                }
            }
            break;
        case FieldType::kDouble: {
            // tag (1) + double size (8)
            if (field_desc.IsRepeated()) {
                size += (1 + 8) * field_count;
            } else {
                double value = reflection->GetDouble(&message, tag);
                if (value != 0.0f) {
                    size += 1 + 8;
                }
            }
            break;
        }
        case FieldType::kFloat: {
            // tag (1) + float size (4)
            if (field_desc.IsRepeated()) {
                size += (1 + 4) * field_count;
            } else {
                float value = reflection->GetFloat(&message, tag);
                if (value != 0.0) {
                    size += 1 + 4;
                }
            }
            break;
        }
        case FieldType::kFixed32: {
            // tag (1) + int32 size (4)
            if (field_desc.IsRepeated()) {
                size += (1 + 4) * field_count;
            } else {
                uint32_t value = reflection->GetFixed32(&message, tag);
                if (value != 0) {
                    size += 1 + 4;
                }
            }
            break;
        }
        case FieldType::kSfixed32: {
            // tag (1) + int32 size (4)
            if (field_desc.IsRepeated()) {
                size += (1 + 4) * field_count;
            } else {
                int32_t value = reflection->GetSfixed32(&message, tag);
                if (value != 0) {
                    size += 1 + 4;
                }
            }
            break;
        }
        case FieldType::kFixed64: {
            // tag (1) + int64 size (8)
            if (field_desc.IsRepeated()) {
                size += (1 + 8) * field_count;
            } else {
                int32_t value = reflection->GetFixed64(&message, tag);
                if (value != 0) {
                    size += 1 + 8;
                }
            }
            break;
        }
        case FieldType::kSfixed64: {
            // tag (1) + int32 size (8)
            if (field_desc.IsRepeated()) {
                size += (1 + 8) * field_count;
            } else {
                int64_t value = reflection->GetSfixed64(&message, tag);
                if (value != 0) {
                    size += 1 + 8;
                }
            }
            break;
        }
        case FieldType::kBytes:
        case FieldType::kGroup:
            cerr << "TODO: Implement FieldType: " << field_desc.GetType()
                 << endl;
            break;
        case kUnknown:
            cerr << "Unknown field type specified which should never be happen "
                    "though"
                 << endl;
            size = 0;
            break;
    }
    return size;
}

size_t ComputeEncodedSize(
        const Message& message,
        const Reflection* reflection,
        const Descriptor* descriptor) {
    size_t size = 0;
    for (const FieldDescriptor& field_desc : descriptor->GetFields()) {
        size += ComputeEncodedFieldSize(message, reflection, field_desc);
    }
    return size;
}

size_t ComputeEncodedSize(const Message& message) {
    return ComputeEncodedSize(
            message, message.GetReflection(), message.GetDescriptor());
}

bool EncodeTag(CodedOutputStream& stream, const FieldDescriptor& field_desc) {
    uint32_t field_number = field_desc.GetFieldNumber();
    WireType wire_type = GetWireType(field_desc.GetType());
    uint32_t tag = (field_number << 3) | wire_type;
    return stream.WriteVarint32(tag);
}

template <typename T, typename EncT>
bool EncodeFieldImpl(
        CodedOutputStream& stream,
        const Message& message,
        const Reflection* reflection,
        const FieldDescriptor& field_desc,
        T (Reflection::*p_get)(const Message*, uint32_t) const,
        T (Reflection::*p_get_repeated)(const Message*, uint32_t, size_t) const,
        bool (CodedOutputStream::*p_write)(EncT)) {
    uint32_t tag = field_desc.GetFieldNumber();
    if (field_desc.IsRepeated()) {
        // Treat enum values as int since we don't know the enum type
        // here
        size_t field_count = reflection->FieldSize(&message, tag);
        for (size_t i = 0; i < field_count; i++) {
            T value = (reflection->*p_get_repeated)(&message, tag, i);
            if (!EncodeTag(stream, field_desc) ||
                !(stream.*p_write)(MemcpyCast<T, EncT>(value))) {
                return false;
            }
        }
    } else {
        T value = (reflection->*p_get)(&message, tag);
        if (value != T()) {
            if (!EncodeTag(stream, field_desc) ||
                !(stream.*p_write)(MemcpyCast<T, EncT>(value))) {
                return false;
            }
        }
    }
    return true;
}

bool EncodeField(
        CodedOutputStream& stream,
        const Message& message,
        const Reflection* reflection,
        const FieldDescriptor& field_desc) {
    uint32_t tag = field_desc.GetFieldNumber();
    switch (field_desc.GetType()) {
        case FieldType::kInt32:
            return EncodeFieldImpl<int32_t, uint32_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetInt32,
                    &Reflection::GetRepeatedInt32,
                    &CodedOutputStream::WriteVarint32);
        case FieldType::kUint32:
            return EncodeFieldImpl<uint32_t, uint32_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetUint32,
                    &Reflection::GetRepeatedUint32,
                    &CodedOutputStream::WriteVarint32);
            break;
        case FieldType::kInt64:
            return EncodeFieldImpl<int64_t, uint64_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetInt64,
                    &Reflection::GetRepeatedInt64,
                    &CodedOutputStream::WriteVarint64);

        case FieldType::kUint64:
            return EncodeFieldImpl<uint64_t, uint64_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetUint64,
                    &Reflection::GetRepeatedUint64,
                    &CodedOutputStream::WriteVarint64);
        case FieldType::kString:
            if (field_desc.IsRepeated()) {
                size_t field_count = reflection->FieldSize(&message, tag);
                for (size_t i = 0; i < field_count; i++) {
                    const string& value =
                            reflection->GetRepeatedString(&message, tag, i);
                    // tag
                    if (!EncodeTag(stream, field_desc) ||
                        // LEN
                        !stream.WriteVarint32(value.size()) ||
                        // value
                        !stream.WriteString(value)) {
                        return false;
                    }
                }
            } else {
                // TODO: Implement hasser in reflection and use it to
                // prevent memory allocation here.
                string value = reflection->GetString(&message, tag);
                if (!value.empty()) {
                    // tag
                    if (!EncodeTag(stream, field_desc) ||
                        // LEN
                        !stream.WriteVarint32(value.size()) ||
                        // value
                        !stream.WriteString(value)) {
                        return false;
                    }
                }
            }
            break;
        case FieldType::kEnum:
            // Treat enum values as int since we don't know the enum type
            // here
            return EncodeFieldImpl<int, uint64_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetEnumValue,
                    &Reflection::GetRepeatedEnumValue,
                    &CodedOutputStream::WriteVarint64);
        case FieldType::kMessage:
            if (field_desc.IsRepeated()) {
                size_t field_count = reflection->FieldSize(&message, tag);
                for (size_t i = 0; i < field_count; i++) {
                    const Message& value =
                            reflection->GetRepeatedMessage(&message, tag, i);
                    size_t sub_msg_size = ComputeEncodedSize(value);
                    // tag
                    if (!EncodeTag(stream, field_desc) ||
                        // LEN
                        !stream.WriteVarint32(sub_msg_size) ||
                        // value
                        !EncodeMessage(
                                stream,
                                value,
                                value.GetReflection(),
                                value.GetDescriptor())) {
                        return false;
                    }
                }
            } else {
                const Message& value = reflection->GetMessage(&message, tag);
                size_t sub_msg_size = ComputeEncodedSize(value);
                if (sub_msg_size > 0) {
                    // Encode only if the sub message is non-default value
                    // tag
                    if (!EncodeTag(stream, field_desc) ||
                        // LEN
                        !stream.WriteVarint32(sub_msg_size) ||
                        // value
                        !EncodeMessage(
                                stream,
                                value,
                                value.GetReflection(),
                                value.GetDescriptor())) {
                        return false;
                    }
                }
            }
            break;
        case kDouble:
            return EncodeFieldImpl<double, uint64_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetDouble,
                    &Reflection::GetRepeatedDouble,
                    &CodedOutputStream::WriteFixedInt64);
        case kFloat:
            return EncodeFieldImpl<float, uint32_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetFloat,
                    &Reflection::GetRepeatedFloat,
                    &CodedOutputStream::WriteFixedInt32);
        case kSint32:
            return EncodeFieldImpl<int32_t, int32_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetSint32,
                    &Reflection::GetRepeatedSint32,
                    &CodedOutputStream::WriteSignedVarint32);
        case kSint64:
            return EncodeFieldImpl<int64_t, int64_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetSint64,
                    &Reflection::GetRepeatedSint64,
                    &CodedOutputStream::WriteSignedVarint64);
        case kFixed32:
            return EncodeFieldImpl<uint32_t, uint32_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetFixed32,
                    &Reflection::GetRepeatedFixed32,
                    &CodedOutputStream::WriteFixedInt32);
        case kFixed64:
            return EncodeFieldImpl<uint64_t, uint64_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetFixed64,
                    &Reflection::GetRepeatedFixed64,
                    &CodedOutputStream::WriteFixedInt64);
        case kSfixed32:
            return EncodeFieldImpl<int32_t, uint32_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetSfixed32,
                    &Reflection::GetRepeatedSfixed32,
                    &CodedOutputStream::WriteFixedInt32);
        case kSfixed64:
            return EncodeFieldImpl<int64_t, uint64_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetSfixed64,
                    &Reflection::GetRepeatedSfixed64,
                    &CodedOutputStream::WriteFixedInt64);
        case kBool:
            return EncodeFieldImpl<bool, uint32_t>(
                    stream,
                    message,
                    reflection,
                    field_desc,
                    &Reflection::GetBool,
                    &Reflection::GetRepeatedBool,
                    &CodedOutputStream::WriteVarint32);
        case kBytes:
        case kGroup:
            cerr << "TODO: Implement encoder for FieldType: "
                 << field_desc.GetType() << endl;
            return false;
        case kUnknown:
            cerr << "Unknown field type specified which should never be "
                    "happen "
                    "though"
                 << endl;
            return false;
    }
    return true;
}

bool EncodeMessage(
        CodedOutputStream& stream,
        const Message& message,
        const Reflection* reflection,
        const Descriptor* descriptor) {
    for (const FieldDescriptor& field_desc : descriptor->GetFields()) {
        if (!EncodeField(stream, message, reflection, field_desc)) {
            cerr << "encode field failed: " << field_desc.GetFieldNumber()
                 << endl;
        }
    }
    return true;
}

bool EncodeMessage(
        OutputStream& stream, const Message& message, size_t& written_size) {
    CodedOutputStream cos(&stream);
    bool result = EncodeMessage(
            cos, message, message.GetReflection(), message.GetDescriptor());
    written_size = cos.WrittenSize();
    return result;
}

}  // namespace decaproto