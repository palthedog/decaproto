#include "fake_message.h"

using namespace decaproto;
using namespace std;

bool FakeMessage::EncodeImpl(decaproto::CodedOutputStream& stream) const {
    if (num_ != 0) {
        stream.WriteTag(kOtherNumTag, decaproto::WireType::kVarint);
        stream.WriteVarint32(num_);
    }

    if (!str_.empty()) {
        stream.WriteTag(kStrTag, decaproto::WireType::kLen);
        stream.WriteVarint32(str_.size());
        stream.WriteString(str_);
    }

    if (has_other_) {
        stream.WriteTag(kOtherTag, decaproto::WireType::kLen);
        size_t size = other_->ComputeEncodedSize();
        stream.WriteVarint32(size);
        other_->EncodeImpl(stream);
    }

    if (enum_field_ != FakeEnum()) {
        stream.WriteTag(kEnumFieldTag, decaproto::WireType::kVarint);
        stream.WriteVarint32(static_cast<uint32_t>(enum_field_));
    }

    for (uint32_t num : rep_nums_) {
        stream.WriteTag(kRepNumsTag, decaproto::WireType::kVarint);
        stream.WriteVarint32(num);
    }

    for (FakeEnum e : rep_enums_) {
        stream.WriteTag(kRepEnumsTag, decaproto::WireType::kVarint);
        stream.WriteVarint32(static_cast<uint32_t>(e));
    }

    return true;
}

Descriptor* kTestDescriptor = nullptr;
Reflection* kTestReflection = nullptr;

const decaproto::Descriptor* FakeMessage::GetDescriptor() const {
    if (kTestDescriptor != nullptr) {
        return kTestDescriptor;
    }

    // Descriptor which represents this Message.
    kTestDescriptor = new Descriptor();
    kTestDescriptor->RegisterField(
            FieldDescriptor(kNumTag, FieldType::kUint32));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kStrTag, FieldType::kString));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kOtherTag, FieldType::kMessage));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kEnumFieldTag, FieldType::kEnum));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kRepNumsTag, FieldType::kUint32, true));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kRepEnumsTag, FieldType::kEnum, true));
    return kTestDescriptor;
}

const Reflection* FakeMessage::GetReflection() const {
    if (kTestReflection != nullptr) {
        return kTestReflection;
    }

    // Descriptor which produces dynamic ways to access the message
    kTestReflection = new Reflection();

    // uint32 num = 1
    kTestReflection->RegisterSetUint32(kNumTag, MsgCast(&FakeMessage::set_num));
    kTestReflection->RegisterGetUint32(kNumTag, MsgCast(&FakeMessage::num));

    // string str = 2
    kTestReflection->RegisterMutableString(
            kStrTag, MsgCast(&FakeMessage::mutable_str));
    kTestReflection->RegisterGetString(kStrTag, MsgCast(&FakeMessage::str));

    // OtherMessage other = 3
    kTestReflection->RegisterMutableMessage(
            kOtherTag, MsgCast(&FakeMessage::mutable_other));
    kTestReflection->RegisterGetMessage(
            kOtherTag, MsgCast(&FakeMessage::other));
    kTestReflection->RegisterHasField(
            kOtherTag, MsgCast(&FakeMessage::has_other));

    // FakeEnum enum_field = 4
    kTestReflection->RegisterSetEnumValue(
            kEnumFieldTag, CastForSetEnumValue(&FakeMessage::set_enum_field));
    kTestReflection->RegisterGetEnumValue(
            kEnumFieldTag, CastForGetEnumValue(&FakeMessage::enum_field));

    // repeated uint32 rep_nums = 5
    kTestReflection->RegisterGetRepeatedUint32(
            kRepNumsTag, MsgCast(&FakeMessage::get_rep_nums));
    kTestReflection->RegisterAddRepeatedUint32(
            kRepNumsTag, MsgCast(&FakeMessage::add_rep_nums));
    kTestReflection->RegisterFieldSize(
            kRepNumsTag, MsgCast(&FakeMessage::rep_nums_size));

    // repeated Fake rep_enums = 6
    kTestReflection->RegisterGetRepeatedEnumValue(
            kRepEnumsTag, MsgCast(&FakeMessage::get_rep_enums));
    kTestReflection->RegisterAddRepeatedEnumValue(
            kRepEnumsTag,
            CastForAddRepeatedEnumValue(&FakeMessage::add_rep_enums));
    kTestReflection->RegisterFieldSize(
            kRepEnumsTag, MsgCast(&FakeMessage::rep_enums_size));

    return kTestReflection;
}

bool FakeOtherMessage::EncodeImpl(decaproto::CodedOutputStream& stream) const {
    if (num_ != 0) {
        stream.WriteTag(kNumTag, decaproto::WireType::kVarint);
        stream.WriteVarint32(num_);
    }

    return true;
}

Descriptor* kFakeOtherDescriptor = nullptr;
Reflection* kFakeOtherReflection = nullptr;
const decaproto::Descriptor* FakeOtherMessage::GetDescriptor() const {
    if (kFakeOtherDescriptor != nullptr) {
        return kFakeOtherDescriptor;
    }

    // Descriptor which represents this Message.
    kFakeOtherDescriptor = new Descriptor();
    kFakeOtherDescriptor->RegisterField(
            FieldDescriptor(kOtherNumTag, FieldType::kUint32));
    return kFakeOtherDescriptor;
}

const Reflection* FakeOtherMessage::GetReflection() const {
    if (kFakeOtherReflection != nullptr) {
        return kFakeOtherReflection;
    }

    // Descriptor which produces dynamic ways to access the message
    kFakeOtherReflection = new Reflection();

    // uint32 num = 1
    kFakeOtherReflection->RegisterSetUint32(
            kOtherNumTag, MsgCast(&FakeOtherMessage::set_num));
    kFakeOtherReflection->RegisterGetUint32(
            kOtherNumTag, MsgCast(&FakeOtherMessage::num));
    return kFakeOtherReflection;
}