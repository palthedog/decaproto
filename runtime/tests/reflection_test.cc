#include "decaproto/reflection.h"

#include <gtest/gtest.h>

#include <string>

#include "decaproto/descriptor.h"
#include "decaproto/message.h"

using namespace decaproto;
using namespace std;

Descriptor* kTestDescriptor = nullptr;
Reflection* kTestReflection = nullptr;

class TestReflection;

const int kNumTag = 0;
const int kStrTag = 1;
class ReflectionTestMessage : public Message {
    uint32_t num_;  // uint32 num = 0
    string str_;    // string str = 1

public:
    ReflectionTestMessage() {
    }

    ~ReflectionTestMessage() {
    }

    void set_num(uint32_t num) {
        num_ = num;
    }

    uint32_t num() {
        return num_;
    }

    void set_str(const string& str) {
        str_ = str;
    }

    const std::string& str() {
        return str_;
    }

    const Descriptor* GetDescriptor() const override {
        if (kTestDescriptor != nullptr) {
            return kTestDescriptor;
        }

        // Descriptor which represents this Message.
        kTestDescriptor = new Descriptor();
        kTestDescriptor->AddField(FieldDescriptor(kNumTag, FieldType::kUint32));
        kTestDescriptor->AddField(FieldDescriptor(kStrTag, FieldType::kString));
        return kTestDescriptor;
    }

    const Reflection* GetReflection() const override;
};

class TestReflection : public Reflection {
public:
    void SetUInt32(
        Message* base_message,
        const FieldDescriptor* field,
        uint32_t value) const override {
        ReflectionTestMessage* message =
            static_cast<ReflectionTestMessage*>(base_message);

        EXPECT_EQ(kUint32, field->GetType());

        if (field->GetTag() == kNumTag) {
            message->set_num(value);
        } else {
            FAIL() << "Invalid field tag" << field->GetTag();
        }
    }

    void SetString(
        Message* base_message,
        const FieldDescriptor* field,
        const string& value) const override {
        ReflectionTestMessage* message =
            static_cast<ReflectionTestMessage*>(base_message);

        EXPECT_EQ(kString, field->GetType());

        if (field->GetTag() == kStrTag) {
            message->set_str(value);
        } else {
            FAIL() << "Invalid field tag" << field->GetTag();
        }
    }
};

const Reflection* ReflectionTestMessage::GetReflection() const {
    if (kTestReflection == nullptr) {
        kTestReflection = new TestReflection();
    }
    return kTestReflection;
}

TEST(ReflectionTest, SimpleTest) {
    ReflectionTestMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    // Note that the order of fields in the descriptor is not guaranteed.

    // There are 2 fields in the message.
    EXPECT_EQ(2, descriptor->GetFields().size());

    for (const FieldDescriptor& field : descriptor->GetFields()) {
        if (field.GetTag() == 0) {
            reflection->SetUInt32(&m, &field, 100);
        } else if (field.GetTag() == 1) {
            reflection->SetString(&m, &field, "hello");
        } else {
            FAIL() << "unexpected field tag: " << field.GetTag();
        }
    }

    EXPECT_EQ(100, m.num());
    EXPECT_EQ("hello", m.str());
}
