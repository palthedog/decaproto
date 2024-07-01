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

const uint32_t kNumTag = 1;
const uint32_t kStrTag = 2;
const uint32_t kEnumTag = 3;
const uint32_t kMessageTag = 4;

// Tests generated Descriptor
// See simple.proto for the definition of SimpleMessage
TEST(ReflectionTest, DescriptorTest) {
    SimpleMessage m;

    const Reflection* reflection = m.GetReflection();
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
            FindFieldDescriptor(descriptor, kEnumTag).GetType());
    EXPECT_EQ(
            FieldType::kMessage,
            FindFieldDescriptor(descriptor, kMessageTag).GetType());
}

// Tests generated Descriptor
// See simple.proto for the definition of SimpleMessage
TEST(ReflectionTest, AccessNumTest) {
    SimpleMessage m;

    // Easy way
    EXPECT_EQ(0, m.num());

    const Reflection* reflection = m.GetReflection();
    const Descriptor* descriptor = m.GetDescriptor();

    EXPECT_EQ(0, reflection->GetInt32(&m, kNumTag));

    // Set num through reflection
    reflection->SetInt32(&m, kNumTag, 100);

    EXPECT_EQ(100, reflection->GetInt32(&m, kNumTag));
}
