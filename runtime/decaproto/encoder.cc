#include "decaproto/encoder.h"

#include <iostream>

#include "decaproto/descriptor.h"
#include "decaproto/reflection.h"

namespace decaproto {

using namespace std;

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

    uint32_t tag = field_desc.GetTag();
    switch (field_desc.GetType()) {
        case FieldType::kInt32:
            if (field_desc.IsRepeated()) {
                for (int32_t value :
                     reflection->GetRepeatedRef<int32_t>(&message, tag)) {
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
        case FieldType::kUInt32:
            if (field_desc.IsRepeated()) {
                for (uint32_t value :
                     reflection->GetRepeatedRef<uint32_t>(&message, tag)) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                uint32_t value = reflection->GetUInt32(&message, tag);
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kInt64:
            if (field_desc.IsRepeated()) {
                for (int64_t value :
                     reflection->GetRepeatedRef<int64_t>(&message, tag)) {
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
        case FieldType::kUInt64:
            if (field_desc.IsRepeated()) {
                for (int64_t value :
                     reflection->GetRepeatedRef<uint64_t>(&message, tag)) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            } else {
                uint64_t value = reflection->GetUInt64(&message, tag);
                if (value != 0) {
                    size += 1;  // tag
                    size += ComputeEncodedVarintSize(value);
                }
            }
            break;
        case FieldType::kString:
            if (field_desc.IsRepeated()) {
                for (const string& value :
                     reflection->GetRepeatedRef<string>(&message, tag)) {
                    // tag
                    size += 1;
                    // LEN
                    size += ComputeEncodedVarintSize(value.size());
                    // value
                    size += value.size();
                }
            } else {
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
        case FieldType::kMessage:

            break;
        default:
            cerr << "Unsupported field type: " << field_desc.GetType() << endl;
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
        cerr << "field: " << field_desc.GetTag() << endl;
        cerr << "field: " << field_desc.GetType() << endl;
        size += ComputeEncodedFieldSize(message, reflection, field_desc);
        cerr << "total size: " << size << endl;
    }
    return size;
}

size_t ComputeEncodedSize(const Message& message) {
    return ComputeEncodedSize(
            message, message.GetReflection(), message.GetDescriptor());
}

bool EncodeMessage(OutputStream& stream, const Message& message) {
    return false;
}

}  // namespace decaproto