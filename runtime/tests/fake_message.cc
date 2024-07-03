#include "fake_message.h"

using namespace decaproto;
using namespace std;

Descriptor* kTestDescriptor = nullptr;
Reflection* kTestReflection = nullptr;

const decaproto::Descriptor* FakeMessage::GetDescriptor() const {
    if (kTestDescriptor != nullptr) {
        return kTestDescriptor;
    }

    // Descriptor which represents this Message.
    kTestDescriptor = new Descriptor();
    kTestDescriptor->RegisterField(
            FieldDescriptor(kNumTag, FieldType::kUInt32));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kStrTag, FieldType::kString));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kOtherTag, FieldType::kMessage));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kEnumFieldTag, FieldType::kEnum));
    kTestDescriptor->RegisterField(
            FieldDescriptor(kRepNumsTag, FieldType::kUInt32, true));
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
    kTestReflection->RegisterSetUInt32(kNumTag, MsgCast(&FakeMessage::set_num));
    kTestReflection->RegisterGetUInt32(kNumTag, MsgCast(&FakeMessage::num));

    // string str = 2
    kTestReflection->RegisterMutableString(
            kStrTag, MsgCast(&FakeMessage::mutable_str));
    kTestReflection->RegisterGetString(kStrTag, MsgCast(&FakeMessage::str));

    // OtherMessage other = 3
    kTestReflection->RegisterMutableMessage(
            kOtherTag, MsgCast(&FakeMessage::mutable_other));
    kTestReflection->RegisterGetMessage(
            kOtherTag, MsgCast(&FakeMessage::other));

    // FakeEnum enum_field = 4
    kTestReflection->RegisterSetEnumValue(
            kEnumFieldTag, CastForSetEnumValue(&FakeMessage::set_enum_field));
    kTestReflection->RegisterGetEnumValue(
            kEnumFieldTag, CastForGetEnumValue(&FakeMessage::enum_field));

    // repeated uint32 rep_nums = 5
    kTestReflection->RegisterGetRepeatedUInt32(
            kRepNumsTag, MsgCast(&FakeMessage::get_rep_nums));
    // kTestReflection->RegisterSetRepeatedUInt32(
    // kRepNumsTag, MsgCast(&FakeMessage::set_rep_nums));
    kTestReflection->RegisterAddRepeatedUInt32(
            kRepNumsTag, MsgCast(&FakeMessage::add_rep_nums));
    kTestReflection->RegisterFieldSize(
            kRepNumsTag, MsgCast(&FakeMessage::rep_nums_size));

    // repeated Fake rep_enums = 6
    kTestReflection->RegisterGetRepeatedEnumValue(
            kRepEnumsTag, MsgCast(&FakeMessage::get_rep_enums));
    /*
kTestReflection->RegisterSetRepeatedEnumValue(
    kRepEnumsTag,
    CastForSetRepeatedEnumValue(&FakeMessage::set_rep_enums));
    */
    kTestReflection->RegisterAddRepeatedEnumValue(
            kRepEnumsTag,
            CastForAddRepeatedEnumValue(&FakeMessage::add_rep_enums));
    kTestReflection->RegisterFieldSize(
            kRepEnumsTag, MsgCast(&FakeMessage::rep_enums_size));

    return kTestReflection;
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
            FieldDescriptor(kOtherNumTag, FieldType::kUInt32));
    return kFakeOtherDescriptor;
}

const Reflection* FakeOtherMessage::GetReflection() const {
    if (kFakeOtherReflection != nullptr) {
        return kFakeOtherReflection;
    }

    // Descriptor which produces dynamic ways to access the message
    kFakeOtherReflection = new Reflection();

    // uint32 num = 1
    kFakeOtherReflection->RegisterSetUInt32(
            kOtherNumTag, MsgCast(&FakeOtherMessage::set_num));
    kFakeOtherReflection->RegisterGetUInt32(
            kOtherNumTag, MsgCast(&FakeOtherMessage::num));
    return kFakeOtherReflection;
}