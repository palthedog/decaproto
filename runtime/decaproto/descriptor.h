#ifndef DECAPROTO_DESCRIPTOR_H
#define DECAPROTO_DESCRIPTOR_H

#include <algorithm>
#include <cstdint>
#include <vector>

namespace decaproto {

// https://protobuf.dev/programming-guides/proto3/#scalar
enum FieldType {
    kUnknown = 0,

    kDouble = 1,
    kFloat = 2,

    kInt32 = 3,
    kInt64 = 4,
    kUint32 = 5,
    kUint64 = 6,
    kSint32 = 7,
    kSint64 = 8,

    kFixed32 = 9,
    kFixed64 = 10,
    kSfixed32 = 11,
    kSfixed64 = 12,

    kBool = 13,
    kString = 14,
    kBytes = 15,

    kEnum = 16,
    kMessage = 17,
    kGroup = 18,
};

// Descriptor for decaproto fields in messages
class FieldDescriptor final {
    uint32_t field_number_;
    FieldType type_;

    bool repeated_;
    bool packed_;

public:
    // Primitive types
    FieldDescriptor(
            uint32_t field_number,
            FieldType type,
            bool repeated = false,
            bool packed = false)
        : field_number_(field_number),
          type_(type),
          repeated_(repeated),
          packed_(packed) {
    }

    ~FieldDescriptor() {
    }

    inline uint32_t GetFieldNumber() const {
        return field_number_;
    }

    inline FieldType GetType() const {
        return type_;
    }

    inline bool IsRepeated() const {
        return repeated_;
    }

    inline bool IsPacked() const {
        return packed_;
    }
};

// Descriptor for decaproto messages
// There are singletons for each message type which is accessible through
// YourMessage::GetDescriptor()
class Descriptor final {
    std::vector<FieldDescriptor> fields_;

public:
    Descriptor() {
    }

    ~Descriptor() {
    }

    void RegisterField(const FieldDescriptor& field) {
        fields_.push_back(field);
    }

    const std::vector<FieldDescriptor>& GetFields() const {
        return fields_;
    }

    const FieldDescriptor* FindFieldByNumber(uint32_t field_number) const {
        auto f =
                find_if(fields_.begin(),
                        fields_.end(),
                        [=](const FieldDescriptor& field) {
                            return field.GetFieldNumber() == field_number;
                        });
        if (f == fields_.end()) {
            return nullptr;
        }
        return &(*f);
    }
};

}  // namespace decaproto

#endif
