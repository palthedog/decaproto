#ifndef FAKE_MESSAGE_H
#define FAKE_MESSAGE_H

#include <string>
#include <vector>

#include "decaproto/descriptor.h"
#include "decaproto/field.h"
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
    FakeOtherMessage() : num_(0) {
    }

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
const int kRepEnumsTag = 6;
class FakeMessage : public decaproto::Message {
    // uint32 num = 1
    uint32_t num_;

    // string str = 2
    std::string str_;

    // OtherMessage other = 3
    mutable decaproto::SubMessagePtr<FakeOtherMessage> other_;
    bool has_other_;

    // FakeEnum enum_field= 4
    FakeEnum enum_field_;
    bool has_enum_field_ = false;

    // repeated uint32 rep_nums = 5
    std::vector<uint32_t> rep_nums_;

    // repeated uint32 rep_nums = 6
    std::vector<FakeEnum> rep_enums_;

public:
    FakeMessage()
        : num_(0),
          str_(""),
          other_(nullptr),
          has_other_(false),
          enum_field_(FakeEnum::UNKNOWN) {
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

    std::string* mutable_str() {
        return &str_;
    }

    bool has_other() {
        return has_other_;
    }

    const FakeOtherMessage& other() const {
        if (!other_) {
            other_.resetDefault();
        }
        return *other_;
    }

    FakeOtherMessage* mutable_other() {
        if (!other_) {
            other_.resetDefault();
        }
        has_other_ = true;
        return other_.get();
    }

    void clear_other() {
        other_.reset();
        has_other_ = false;
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

    uint32_t get_rep_nums(size_t index) const {
        return rep_nums_[index];
    }

    void set_rep_nums(size_t index, uint32_t value) {
        rep_nums_[index] = value;
    }

    uint32_t* add_rep_nums() {
        rep_nums_.push_back(0);
        return &rep_nums_.back();
    }

    size_t rep_nums_size() const {
        return rep_nums_.size();
    }

    const std::vector<FakeEnum>& rep_enums() const {
        return rep_enums_;
    }

    std::vector<FakeEnum>* mutable_rep_enums() {
        return &rep_enums_;
    }

    uint32_t get_rep_enums(size_t index) const {
        return rep_enums_[index];
    }

    void set_rep_enums(size_t index, FakeEnum value) {
        rep_enums_[index] = value;
    }

    FakeEnum* add_rep_enums() {
        rep_enums_.push_back(FakeEnum());
        return &rep_enums_.back();
    }

    size_t rep_enums_size() const {
        return rep_enums_.size();
    }

    const decaproto::Descriptor* GetDescriptor() const override;
    const decaproto::Reflection* GetReflection() const override;
};

#endif  // FAKE_MESSAGE_H
