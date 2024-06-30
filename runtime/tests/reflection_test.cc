#include "decaproto/reflection.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "decaproto/descriptor.h"
#include "decaproto/message.h"
#include "decaproto/reflection.h"
#include "decaproto/reflection_util.h"

using namespace decaproto;
using namespace std;

Descriptor* kTestDescriptor = nullptr;
Reflection* kTestReflection = nullptr;

enum TestEnum {
    UNKNOWN = 0,
    ENUM_A = 1,
    ENUM_B = 2,
    ENUM_C = 3,
};

class OtherMessage : public Message {
    uint32_t num_;

public:
    uint32_t num() const {
        return num_;
    }

    void set_num(uint32_t num) {
        num_ = num;
    }

    // We don't use following methods in this test.
    const Descriptor* GetDescriptor() const override {
        return nullptr;
    }
    const Reflection* GetReflection() const override {
        return nullptr;
    }
};

const int kNumTag = 0;
const int kStrTag = 1;
const int kOtherTag = 2;
const int kEnumFieldTag = 3;
const int kRepNumsTag = 4;
class ReflectionTestMessage : public Message {
    // uint32 num = 0
    uint32_t num_;

    // string str = 1
    string str_;

    // OtherMessage other = 2
    unique_ptr<OtherMessage> other_;

    // TestEnum enum_field= 3
    TestEnum enum_field_;
    bool has_enum_field_ = false;

    // repeated uint32 rep_nums = 4
    vector<uint32_t> rep_nums_;

public:
    ReflectionTestMessage() {
    }

    ~ReflectionTestMessage() {
    }

    void set_num(uint32_t num) {
        num_ = num;
    }

    uint32_t num() const {
        return num_;
    }

    void set_str(const string& str) {
        str_ = str;
    }

    const std::string& str() const {
        return str_;
    }

    bool has_other() {
        return other_.get() != nullptr;
    }

    const OtherMessage& other() const {
        return *other_;
    }

    OtherMessage* mutable_other() {
        if (!other_) {
            other_ = std::make_unique<OtherMessage>();
        }
        return other_.get();
    }

    const TestEnum& enum_field() const {
        return enum_field_;
    }

    void set_enum_field(TestEnum enum_field) {
        enum_field_ = enum_field;
        has_enum_field_ = true;
    }

    bool has_enum_field() {
        return has_enum_field_;
    }

    const Descriptor* GetDescriptor() const override {
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

    const Reflection* GetReflection() const override {
        if (kTestReflection != nullptr) {
            return kTestReflection;
        }

        // Descriptor which produces dynamic ways to access the message
        kTestReflection = new Reflection();

        // uint32 num = 1
        kTestReflection->RegisterSetUInt32(
                kNumTag, MsgCast(&ReflectionTestMessage::set_num));
        kTestReflection->RegisterGetUInt32(
                kNumTag, MsgCast(&ReflectionTestMessage::num));

        // string str = 2
        kTestReflection->RegisterSetString(
                kStrTag, MsgCast(&ReflectionTestMessage::set_str));
        kTestReflection->RegisterGetString(
                kStrTag, MsgCast(&ReflectionTestMessage::str));

        // OtherMessage other = 3
        kTestReflection->RegisterMutableMessage(
                kOtherTag, MsgCast(&ReflectionTestMessage::mutable_other));
        kTestReflection->RegisterGetMessage(
                kOtherTag, MsgCast(&ReflectionTestMessage::other));

        // TestEnum enum_field = 4
        kTestReflection->RegisterSetEnumValue(
                kEnumFieldTag,
                CastForSetEnumValue(&ReflectionTestMessage::set_enum_field));
        kTestReflection->RegisterGetEnumValue(
                kEnumFieldTag,
                CastForGetEnumValue(&ReflectionTestMessage::enum_field));
        return kTestReflection;
    }
};

const FieldDescriptor& FindFieldDescriptor(
        const Descriptor* descriptor, uint32_t tag) {
    return *std::find_if(
            descriptor->GetFields().begin(),
            descriptor->GetFields().end(),
            [tag](const FieldDescriptor& field_desc) {
                return field_desc.GetTag() == tag;
            });
}

TEST(ReflectionTest, SimpleTest) {
    ReflectionTestMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    const FieldDescriptor& num_field = FindFieldDescriptor(descriptor, kNumTag);
    reflection->SetUInt32(&m, num_field.GetTag(), 100);
    const FieldDescriptor& str_field = FindFieldDescriptor(descriptor, kStrTag);
    reflection->SetString(&m, str_field.GetTag(), "hello");

    EXPECT_EQ(100, m.num());
    EXPECT_EQ("hello", m.str());
}

TEST(ReflectionTest, MessageFieldTest) {
    ReflectionTestMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    // Note that the order of fields in the descriptor is not guaranteed.

    // m doesn't have other field yet
    EXPECT_FALSE(m.has_other());

    // Note that the order of fields in the descriptor is not guaranteed.
    const FieldDescriptor& field_desc =
            FindFieldDescriptor(descriptor, kOtherTag);
    EXPECT_EQ(FieldType::kMessage, field_desc.GetType());

    // Get a mutable pointer to the message field through reflection
    OtherMessage* other = static_cast<OtherMessage*>(
            reflection->MutableMessage(&m, field_desc.GetTag()));
    other->set_num(128);

    EXPECT_TRUE(m.has_other());
    EXPECT_EQ(128, m.other().num());
}

TEST(ReflectionTest, SetEnumValueTest) {
    ReflectionTestMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    // Note that the order of fields in the descriptor is not guaranteed.

    // m doesn't have the field yet
    EXPECT_FALSE(m.has_enum_field());

    // Note that the order of fields in the descriptor is not guaranteed.
    const FieldDescriptor& field_desc =
            FindFieldDescriptor(descriptor, kEnumFieldTag);
    EXPECT_EQ(FieldType::kEnum, field_desc.GetType());

    // Get a mutable pointer to the message field through reflection
    reflection->SetEnumValue(&m, field_desc.GetTag(), (int)TestEnum::ENUM_B);

    EXPECT_TRUE(m.has_enum_field());
    EXPECT_EQ(TestEnum::ENUM_B, m.enum_field());
}

TEST(ReflectionTest, GetEnumValueTest) {
    ReflectionTestMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    // Note that the order of fields in the descriptor is not guaranteed.

    // m doesn't have other field yet
    m.set_enum_field(TestEnum::ENUM_C);

    // Note that the order of fields in the descriptor is not guaranteed.
    const FieldDescriptor& field_desc =
            FindFieldDescriptor(descriptor, kEnumFieldTag);
    EXPECT_EQ(FieldType::kEnum, field_desc.GetType());

    // Get a mutable pointer to the message field through reflection
    int enum_value = reflection->GetEnumValue(&m, field_desc.GetTag());

    EXPECT_EQ(TestEnum::ENUM_C, (TestEnum)enum_value);
}

/*
TEST_F(ReflectionTest, GetRepeatedFieldTest) {
    ReflectionTestMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    // Assuming reflection methods to add and get repeated fields exist
    // Add elements through reflection
    reflection->AddRepeatedUint32Field(&message, kRepNumsTag, 10);
    reflection->AddRepeatedUint32Field(&message, kRepNumsTag, 20);
    reflection->AddRepeatedUint32Field(&message, kRepNumsTag, 30);

    // Get and test elements through reflection
    EXPECT_EQ(
            message.GetReflection()->GetRepeatedUint32Field(
                    message, kRepNumsTag, 0),
            10);
    EXPECT_EQ(
            message.GetReflection()->GetRepeatedUint32Field(
                    message, kRepNumsTag, 1),
            20);
    EXPECT_EQ(
            message.GetReflection()->GetRepeatedUint32Field(
                    message, kRepNumsTag, 2),
            30);
}
*/