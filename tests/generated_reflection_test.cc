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

// Test generated Descriptor
// See simple.proto for the definition of SimpleMessage
TEST(ReflectionTest, DescriptorTest) {
    SimpleMessage m;

    const Reflection* reflection = m.GetReflection();
    const Descriptor* descriptor = m.GetDescriptor();

    EXPECT_EQ(4, descriptor->GetFields().size());

    EXPECT_EQ(FieldType::kInt32, FindFieldDescriptor(descriptor, 1).GetType());
    EXPECT_EQ(FieldType::kString, FindFieldDescriptor(descriptor, 2).GetType());
    EXPECT_EQ(FieldType::kEnum, FindFieldDescriptor(descriptor, 3).GetType());
    EXPECT_EQ(
            FieldType::kMessage, FindFieldDescriptor(descriptor, 4).GetType());
}
