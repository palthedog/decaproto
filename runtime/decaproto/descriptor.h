#ifndef DECAPROTO_DESCRIPTOR_H
#define DECAPROTO_DESCRIPTOR_H

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

class FieldDescriptor {
    uint32_t tag_;
    FieldType type_;

    bool repeated_;
    bool packed_;

    // Member pointer to a field of Message class.
    // Member pointer to a field of Message class.
    //(void*)Message::*field_ptr_;

public:
    FieldDescriptor() {
    }

    // Primitive types
    FieldDescriptor(
        uint32_t tag,
        FieldType type,
        bool repeated = false,
        bool packed = false)
        : tag_(tag), type_(type), repeated_(repeated), packed_(packed) {
    }

    ~FieldDescriptor() {
    }

    inline uint32_t GetTag() const {
        return tag_;
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

class MessageDescriptor {
    std::vector<FieldDescriptor> fields_;

public:
    MessageDescriptor() {
    }

    ~MessageDescriptor() {
    }

    void AddField(const FieldDescriptor& field) {
        fields_.push_back(field);
    }

    const std::vector<FieldDescriptor>& GetFields() const {
        return fields_;
    }
};

}  // namespace decaproto

#endif
