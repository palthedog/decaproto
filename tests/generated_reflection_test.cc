#include <gtest/gtest.h>

#include <algorithm>

#include "tests/repeated.pb.h"
#include "tests/simple.pb.h"

using namespace decaproto;
using namespace std;

const FieldDescriptor& FindFieldDescriptor(
        const Descriptor* descriptor, uint32_t tag) {
    return *find_if(
            descriptor->GetFields().begin(),
            descriptor->GetFields().end(),
            [=](const FieldDescriptor& field) {
                return field.GetFieldNumber() == tag;
            });
}

// SimpleMesaage(simple.proto) fields
const uint32_t kNumTag = 1;
const uint32_t kStrTag = 2;
const uint32_t kEnumValueTag = 3;
const uint32_t kOtherTag = 4;
const uint32_t kFloatValueTag = 5;
const uint32_t kDoubleValueTag = 6;
const uint32_t kBoolValueTag = 7;

// OtherMessage(simple.proto) fields
const uint32_t kOtherNumTag = 1;

// RepeatedMessage(repeated.proto) fields
const uint32_t kNumsTag = 1;
const uint32_t kStrsTag = 2;
const uint32_t kEnumValuesTag = 3;

// Tests generated Descriptor
// See simple.proto for the definition of SimpleMessage
TEST(ReflectionTest, DescriptorTest) {
    SimpleMessage m;

    const Descriptor* descriptor = m.GetDescriptor();

    EXPECT_EQ(7, descriptor->GetFields().size());

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
    EXPECT_EQ(
            FieldType::kFloat,
            FindFieldDescriptor(descriptor, kFloatValueTag).GetType());
    EXPECT_EQ(
            FieldType::kDouble,
            FindFieldDescriptor(descriptor, kDoubleValueTag).GetType());
    EXPECT_EQ(
            FieldType::kBool,
            FindFieldDescriptor(descriptor, kBoolValueTag).GetType());
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
    *reflection->MutableString(&m, kStrTag) = "udon";

    EXPECT_EQ("udon", reflection->GetString(&m, kStrTag));
}

TEST(ReflectionTest, StringNotCopiedTest) {
    SimpleMessage m;

    const Reflection* reflection = m.GetReflection();

    *reflection->MutableString(&m, kStrTag) = "udon";

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

    const decaproto::Message& other = reflection->GetMessage(&m, kOtherTag);
    const Reflection* other_reflection = other.GetReflection();
    EXPECT_EQ(100, other_reflection->GetInt32(&other, kOtherNumTag));
}

TEST(ReflectionTest, RepeatedInt32Test) {
    RepeatedMessage m;

    const Reflection* reflection = m.GetReflection();

    // Easy way
    EXPECT_EQ(0, m.nums().size());

    *reflection->AddRepeatedInt32(&m, kNumsTag) = 100;
    *reflection->AddRepeatedInt32(&m, kNumsTag) = 200;
    *reflection->AddRepeatedInt32(&m, kNumsTag) = 300;

    // Check with normal getter
    EXPECT_EQ(3, m.nums().size());
    EXPECT_EQ(100, m.nums()[0]);
    EXPECT_EQ(200, m.nums()[1]);
    EXPECT_EQ(300, m.nums()[2]);

    // Check with reflection getter
    EXPECT_EQ(3, reflection->FieldSize(&m, kNumsTag));
    EXPECT_EQ(100, reflection->GetRepeatedInt32(&m, kNumsTag, 0));
    EXPECT_EQ(200, reflection->GetRepeatedInt32(&m, kNumsTag, 1));
    EXPECT_EQ(300, reflection->GetRepeatedInt32(&m, kNumsTag, 2));
}

TEST(ReflectionTest, RepeatedStringTest) {
    RepeatedMessage m;

    const Reflection* reflection = m.GetReflection();

    // Easy way
    EXPECT_EQ(0, m.strs().size());

    cerr << "start adding strings" << endl;

    *reflection->AddRepeatedString(&m, kStrsTag) = "A";
    *reflection->AddRepeatedString(&m, kStrsTag) = "BB";
    *reflection->AddRepeatedString(&m, kStrsTag) = "CCC";

    cerr << "added 3 strings" << endl;

    // Check with normal getter
    EXPECT_EQ(3, m.strs().size());
    EXPECT_EQ("A", m.strs()[0]);
    EXPECT_EQ("BB", m.strs()[1]);
    EXPECT_EQ("CCC", m.strs()[2]);

    cerr << "checked with standard interface" << endl;

    // Check with reflection getter
    EXPECT_EQ(3, reflection->FieldSize(&m, kStrsTag));

    cerr << "got field size" << endl;

    EXPECT_EQ("A", reflection->GetRepeatedString(&m, kStrsTag, 0));
    EXPECT_EQ("BB", reflection->GetRepeatedString(&m, kStrsTag, 1));
    EXPECT_EQ("CCC", reflection->GetRepeatedString(&m, kStrsTag, 2));

    cerr << "end of test" << endl;
}
/*
TEST(ReflectionTest, RepeatedEnumTest) {
    RepeatedMessage m;

    const Reflection* reflection = m.GetReflection();

    // Easy way
    EXPECT_EQ(0, m.enum_values().size());

    std::vector<RepeatedEnum>* mut_enum_values_ =
            reflection->MutableRepeatedRef<RepeatedEnum>(&m, kEnumValuesTag);
    mut_enum_values_->push_back(REP_ENUM_A);
    mut_enum_values_->push_back(REP_ENUM_B);
    mut_enum_values_->push_back(REP_ENUM_C);

    // Check with normal getter
    EXPECT_EQ(3, m.enum_values().size());
    EXPECT_EQ(REP_ENUM_A, m.enum_values()[0]);
    EXPECT_EQ(REP_ENUM_B, m.enum_values()[1]);
    EXPECT_EQ(REP_ENUM_C, m.enum_values()[2]);

    // Check with reflection getter
    const std::vector<RepeatedEnum>& nums =
            reflection->GetRepeatedRef<RepeatedEnum>(&m, kEnumValuesTag);
    EXPECT_EQ(REP_ENUM_A, nums[0]);
    EXPECT_EQ(REP_ENUM_B, nums[1]);
    EXPECT_EQ(REP_ENUM_C, nums[2]);
}
*/