#ifndef FAKE_MESSAGE_H
#define FAKE_MESSAGE_H

#include <memory>
#include <string>
#include <vector>

#include "decaproto/descriptor.h"
#include "decaproto/message.h"
#include "decaproto/reflection.h"
#include "decaproto/reflection_util.h"

enum FakeEnum {
    UNKNOWN = 0,
    ENUM_A = 1,
    ENUM_B = 2,
    ENUM_C = 3,
};

const int kOtherNumTag = 1;
class FakeOtherMessage : public decaproto::Message {
    uint32_t num_;

public:
    uint32_t num() const {
        return num_;
    }

    void set_num(uint32_t num) {
        num_ = num;
    }

    const decaproto::Descriptor* GetDescriptor() const override;
    const decaproto::Reflection* GetReflection() const override;
};

const int kNumTag = 1;
const int kStrTag = 2;
const int kOtherTag = 3;
const int kEnumFieldTag = 4;
const int kRepNumsTag = 5;
class FakeMessage : public decaproto::Message {
    // uint32 num = 1
    uint32_t num_;

    // string str = 2
    std::string str_;

    // OtherMessage other = 3
    std::unique_ptr<FakeOtherMessage> other_;

    // FakeEnum enum_field= 4
    FakeEnum enum_field_;
    bool has_enum_field_ = false;

    // repeated uint32 rep_nums = 5
    std::vector<uint32_t> rep_nums_;

public:
    FakeMessage() {
    }

    ~FakeMessage() {
    }

    void set_num(uint32_t num) {
        num_ = num;
    }

    uint32_t num() const {
        return num_;
    }

    void set_str(const std::string& str) {
        str_ = str;
    }

    const std::string& str() const {
        return str_;
    }

    bool has_other() {
        return other_.get() != nullptr;
    }

    const FakeOtherMessage& other() const {
        return *other_;
    }

    FakeOtherMessage* mutable_other() {
        if (!other_) {
            other_ = std::make_unique<FakeOtherMessage>();
        }
        return other_.get();
    }

    const FakeEnum& enum_field() const {
        return enum_field_;
    }

    void set_enum_field(FakeEnum enum_field) {
        enum_field_ = enum_field;
        has_enum_field_ = true;
    }

    bool has_enum_field() {
        return has_enum_field_;
    }

    const std::vector<uint32_t>& rep_nums() const {
        return rep_nums_;
    }

    std::vector<uint32_t>* mutable_rep_nums() {
        return &rep_nums_;
    }

    const decaproto::Descriptor* GetDescriptor() const override;
    const decaproto::Reflection* GetReflection() const override;
};

#endif  // FAKE_MESSAGE_H
