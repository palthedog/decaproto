#include <gtest/gtest.h>

#include <algorithm>

#include "tests/simple.pb.h"

using namespace decaproto;
using namespace std;

const FieldDescriptor& FindFieldDescriptor(
        const Descriptor* descriptor, uint32_t tag) {
    return *find_if(
            descriptor->GetFields().begin(),
            descriptor->GetFields().end(),
            [=](const FieldDescriptor& field) {
                return field.GetTag() == tag;
            });
}

// SimpleMesaage fields
const uint32_t kNumTag = 1;
const uint32_t kStrTag = 2;
const uint32_t kEnumValueTag = 3;
const uint32_t kOtherTag = 4;

// OtherMessage field(s)
const uint32_t kOtherNumTag = 1;

// Tests generated Descriptor
// See simple.proto for the definition of SimpleMessage
TEST(ReflectionTest, DescriptorTest) {
    SimpleMessage m;

    const Descriptor* descriptor = m.GetDescriptor();

    EXPECT_EQ(4, descriptor->GetFields().size());

    EXPECT_EQ(
            FieldType::kInt32,
            FindFieldDescriptor(descriptor, kNumTag).GetType());
    EXPECT_EQ(
            FieldType::kString,
            FindFieldDescriptor(descriptor, kStrTag).GetType());
    EXPECT_EQ(
            FieldType::kEnum,
            FindFieldDescriptor(descriptor, kEnumValueTag).GetType());
    EXPECT_EQ(
            FieldType::kMessage,
            FindFieldDescriptor(descriptor, kOtherTag).GetType());
}

TEST(ReflectionTest, AccessNumTest) {
    SimpleMessage m;

    // Easy way
    EXPECT_EQ(0, m.num());

    const Reflection* reflection = m.GetReflection();

    EXPECT_EQ(0, reflection->GetInt32(&m, kNumTag));

    // Set num through reflection
    reflection->SetInt32(&m, kNumTag, 100);

    EXPECT_EQ(100, reflection->GetInt32(&m, kNumTag));
}

TEST(ReflectionTest, AccessStrTest) {
    SimpleMessage m;

    // Easy way
    EXPECT_EQ("", m.str());

    const Reflection* reflection = m.GetReflection();

    EXPECT_EQ("", reflection->GetString(&m, kStrTag));

    // Set num through reflection
    reflection->SetString(&m, kStrTag, "udon");

    EXPECT_EQ("udon", reflection->GetString(&m, kStrTag));
}

TEST(ReflectionTest, StringNotCopiedTest) {
    SimpleMessage m;

    const Reflection* reflection = m.GetReflection();

    reflection->SetString(&m, kStrTag, "udon");

    // Compare pointers to check that both getters don't copy the string
    EXPECT_EQ(&m.str(), &reflection->GetString(&m, kStrTag));
}

TEST(ReflectionTest, EnumTest) {
    SimpleMessage m;

    const Reflection* reflection = m.GetReflection();

    // Easy way
    EXPECT_EQ(0, m.enum_value());

    reflection->SetEnumValue(&m, kEnumValueTag, ENUM_B);

    EXPECT_EQ(ENUM_B, m.enum_value());
    EXPECT_EQ(ENUM_B, reflection->GetEnumValue(&m, kEnumValueTag));
}

TEST(ReflectionTest, MessageTest) {
    SimpleMessage m;

    const Reflection* reflection = m.GetReflection();

    // Easy way
    EXPECT_FALSE(m.has_other());

    OtherMessage* mut_other = static_cast<OtherMessage*>(
            reflection->MutableMessage(&m, kOtherTag));
    mut_other->set_other_num(100);

    EXPECT_EQ(100, m.other().other_num());

    const Message& other = reflection->GetMessage(&m, kOtherTag);
    const Reflection* other_reflection = other.GetReflection();
    EXPECT_EQ(100, other_reflection->GetInt32(&other, kOtherNumTag));
}
