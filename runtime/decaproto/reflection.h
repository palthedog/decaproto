#ifndef DECAPROTO_REFLECTION_H
#define DECAPROTO_REFLECTION_H

#include <cassert>
#include <functional>
#include <map>
#include <string>

#include "decaproto/descriptor.h"

namespace decaproto {

class Message;

class Reflection {
    template <typename T>
    using SetterFn = std::function<void(Message*, T)>;

    template <typename T>
    using GetterFn = std::function<T(const Message*)>;

    template <typename T>
    using MutableFn = std::function<T(Message*)>;

#define DEFINE_FOR(cc_type, CcType)                              \
    std::map<uint32_t, SetterFn<cc_type>> set_##CcType##_impls_; \
    std::map<uint32_t, GetterFn<cc_type>> get_##CcType##_impls_;

    // This is the expanded version of the above macro for reference.
    // std::map<uint32_t, SetterFn<uint32_t>> set_UInt32_impls_;

    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(const std::string&, String)

#undef DEFINE_FOR

    // For Message fields
    std::map<uint32_t, MutableFn<Message*>> mutable_Message_impls_;
    std::map<uint32_t, GetterFn<const Message&>> get_Message_impls_;

    // For Enum fields
    // TODO: Implement EnumValueDescriptor so that callers can access
    // more information about the enum value.
    std::map<uint32_t, SetterFn<int>> set_EnumValue_impls_;
    std::map<uint32_t, GetterFn<int>> get_EnumValue_impls_;

public:
    Reflection() {
    }

    ~Reflection() {
    }

#define DEFINE_FOR(cc_type, CcType)                                          \
    void RegisterSet##CcType(                                                \
            const FieldDescriptor& field, const SetterFn<cc_type>& setter) { \
        uint32_t tag = field.GetTag();                                       \
        set_##CcType##_impls_[tag] = setter;                                 \
    }                                                                        \
                                                                             \
    void Set##CcType(                                                        \
            Message* message, const FieldDescriptor* field, cc_type value)   \
            const {                                                          \
        auto it = set_##CcType##_impls_.find(field->GetTag());               \
        assert(it != set_##CcType##_impls_.end());                           \
        it->second(message, value);                                          \
    }

    // Define Setter resgisterers

    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(const std::string&, String)
    DEFINE_FOR(int, EnumValue)

#undef DEFINE_FOR  // Define Registerer

#define DEFINE_FOR(cc_type, CcType)                                           \
    void RegisterGet##CcType(                                                 \
            const FieldDescriptor& field, const GetterFn<cc_type>& getter) {  \
        uint32_t tag = field.GetTag();                                        \
        get_##CcType##_impls_[tag] = getter;                                  \
    }                                                                         \
                                                                              \
    cc_type Get##CcType(const Message* message, const FieldDescriptor* field) \
            const {                                                           \
        auto it = get_##CcType##_impls_.find(field->GetTag());                \
        assert(it != get_##CcType##_impls_.end());                            \
        return it->second(message);                                           \
    }

    // Define Getter resgisterers

    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(const std::string&, String)
    DEFINE_FOR(const Message&, Message)
    DEFINE_FOR(int, EnumValue)

#undef DEFINE_FOR  // Define Registerer

#define DEFINE_FOR(cc_type, CcType)                  \
    void RegisterMutable##CcType(                    \
            const FieldDescriptor& field,            \
            const MutableFn<cc_type>& mut_getter) {  \
        uint32_t tag = field.GetTag();               \
        mutable_##CcType##_impls_[tag] = mut_getter; \
    }

    DEFINE_FOR(Message*, Message)

#undef DEFINE_FOR  // Define Registerer

    // Returns a mutable field message associated with the
    // FieldDescriptor
    Message* MutableMessage(
            Message* message, const FieldDescriptor* field) const {
        auto it = mutable_Message_impls_.find(field->GetTag());
        assert(it != mutable_Message_impls_.end());
        return it->second(message);
    }
};

}  // namespace decaproto

#endif
