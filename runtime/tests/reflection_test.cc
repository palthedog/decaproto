#include "decaproto/reflection.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "decaproto/descriptor.h"
#include "decaproto/message.h"
#include "decaproto/reflection.h"
#include "decaproto/reflection_util.h"
#include "fake_message.h"

using namespace decaproto;
using namespace std;

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
    FakeMessage m;
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
    FakeMessage m;
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
    FakeOtherMessage* other = static_cast<FakeOtherMessage*>(
            reflection->MutableMessage(&m, field_desc.GetTag()));
    other->set_num(128);

    EXPECT_TRUE(m.has_other());
    EXPECT_EQ(128, m.other().num());
}

TEST(ReflectionTest, SetEnumValueTest) {
    FakeMessage m;
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
    reflection->SetEnumValue(&m, field_desc.GetTag(), (int)FakeEnum::ENUM_B);

    EXPECT_TRUE(m.has_enum_field());
    EXPECT_EQ(FakeEnum::ENUM_B, m.enum_field());
}

TEST(ReflectionTest, GetEnumValueTest) {
    FakeMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    // Note that the order of fields in the descriptor is not guaranteed.

    // m doesn't have other field yet
    m.set_enum_field(FakeEnum::ENUM_C);

    // Note that the order of fields in the descriptor is not guaranteed.
    const FieldDescriptor& field_desc =
            FindFieldDescriptor(descriptor, kEnumFieldTag);
    EXPECT_EQ(FieldType::kEnum, field_desc.GetType());

    // Get a mutable pointer to the message field through reflection
    int enum_value = reflection->GetEnumValue(&m, field_desc.GetTag());

    EXPECT_EQ(FakeEnum::ENUM_C, (FakeEnum)enum_value);
}

TEST(ReflectionTest, GetRepeatedFieldTest) {
    FakeMessage m;
    const Reflection* reflection = m.GetReflection();

    EXPECT_EQ(0, m.rep_nums().size());

    // Assuming reflection methods to add and get repeated fields exist
    // Add elements through reflection
    m.mutable_rep_nums()->push_back(10);
    m.mutable_rep_nums()->push_back(20);
    m.mutable_rep_nums()->push_back(30);

    EXPECT_EQ(3, m.rep_nums().size());

    // Get and test elements through reflection
    const vector<uint32_t>& nums =
            reflection->GetRepeatedRef<uint32_t>(&m, kRepNumsTag);
    EXPECT_EQ(3, nums.size());
    EXPECT_EQ(10, nums[0]);
    EXPECT_EQ(20, nums[1]);
    EXPECT_EQ(30, nums[2]);
}

TEST(ReflectionTest, SetRepeatedFieldTest) {
    FakeMessage m;
    const Reflection* reflection = m.GetReflection();

    EXPECT_EQ(0, m.rep_nums().size());

    // Push values through reflection
    vector<uint32_t>* nums =
            reflection->MutableRepeatedRef<uint32_t>(&m, kRepNumsTag);
    nums->push_back(10);
    nums->push_back(20);
    nums->push_back(30);

    EXPECT_EQ(3, m.rep_nums().size());
    EXPECT_EQ(10, m.rep_nums()[0]);
    EXPECT_EQ(20, m.rep_nums()[1]);
    EXPECT_EQ(30, m.rep_nums()[2]);
}
