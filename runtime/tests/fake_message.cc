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
    kTestReflection->RegisterSetString(kStrTag, MsgCast(&FakeMessage::set_str));
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
    kTestReflection->RegisterMutableRepeatedRef(
            kRepNumsTag, MsgCast(&FakeMessage::mutable_rep_nums));

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