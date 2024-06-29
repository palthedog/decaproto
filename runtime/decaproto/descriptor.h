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
    kUInt32 = 5,
    kUInt64 = 6,
    kSInt32 = 7,
    kSInt64 = 8,

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

class Descriptor {
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
};

}  // namespace decaproto

#endif
