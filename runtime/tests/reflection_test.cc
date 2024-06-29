#include "decaproto/reflection.h"

#include <gtest/gtest.h>

#include <string>

#include "decaproto/descriptor.h"
#include "decaproto/message.h"

using namespace decaproto;
using namespace std;

Descriptor* kTestDescriptor = nullptr;
Reflection* kTestReflection = nullptr;

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
class ReflectionTestMessage : public Message {
    uint32_t num_;                         // uint32 num = 0
    string str_;                           // string str = 1
    std::unique_ptr<OtherMessage> other_;  // OtherMessage other = 2

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

    bool has_other() {
        return other_.get() != nullptr;
    }

    const OtherMessage& other() {
        return *other_;
    }

    OtherMessage* mutable_other() {
        if (!other_) {
            other_ = std::make_unique<OtherMessage>();
        }
        return other_.get();
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
        return kTestDescriptor;
    }

    const Reflection* GetReflection() const override {
        if (kTestReflection != nullptr) {
            return kTestReflection;
        }

        // Descriptor which produces dynamic ways to access the message
        kTestReflection = new Reflection();

        // uint32 num = 1
        kTestReflection->RegisterUInt32Field(
            FieldDescriptor(kNumTag, FieldType::kUInt32),
            [](Message* base_message, uint32_t value) {
                ReflectionTestMessage* message =
                    static_cast<ReflectionTestMessage*>(base_message);
                message->set_num(value);
            });

        // string str = 2
        kTestReflection->RegisterStringField(
            FieldDescriptor(kStrTag, FieldType::kString),
            [](Message* base_message, const string& value) {
                ReflectionTestMessage* message =
                    static_cast<ReflectionTestMessage*>(base_message);
                message->set_str(value);
            });

        // OtherMessage other = 3
        kTestReflection->RegisterMessageField(
            FieldDescriptor(kOtherTag, FieldType::kMessage),
            [](Message* base_message) {
                ReflectionTestMessage* message =
                    static_cast<ReflectionTestMessage*>(base_message);
                return message->mutable_other();
            });

        return kTestReflection;
    }
};

TEST(ReflectionTest, SimpleTest) {
    ReflectionTestMessage m;
    const Descriptor* descriptor = m.GetDescriptor();
    const Reflection* reflection = m.GetReflection();

    for (const FieldDescriptor& field : descriptor->GetFields()) {
        if (field.GetTag() == 0) {
            reflection->SetUInt32(&m, &field, 100);
        } else if (field.GetTag() == 1) {
            reflection->SetString(&m, &field, "hello");
        } else if (field.GetTag() == 2) {
            // other Message field
            // ingnore in this test but tested in other tests
        } else {
            FAIL() << "unexpected field tag: " << field.GetTag();
        }
    }

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
    const FieldDescriptor& field_desc = *std::find_if(
        descriptor->GetFields().begin(),
        descriptor->GetFields().end(),
        [](const FieldDescriptor& field_desc) {
            return field_desc.GetTag() == kOtherTag;
        });
    EXPECT_EQ(FieldType::kMessage, field_desc.GetType());

    // Get a mutable pointer to the message field through reflection
    OtherMessage* other =
        static_cast<OtherMessage*>(reflection->MutableMessage(&m, &field_desc));
    other->set_num(128);

    EXPECT_TRUE(m.has_other());
    EXPECT_EQ(128, m.other().num());
}
