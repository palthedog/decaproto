#ifndef DECAPROTO_REFLECTION_H
#define DECAPROTO_REFLECTION_H

#include <cassert>
#include <functional>
#include <map>
#include <string>

#include "decaproto/descriptor.h"

namespace decaproto {

class Message;

class Reflection final {
    template <typename T>
    using SetterFn = std::function<void(Message*, T)>;

    template <typename T>
    using GetterFn = std::function<T(const Message*)>;

    template <typename T>
    using MutableFn = std::function<T(Message*)>;

    // TODO: Consider adding/using a single field accessor to implement both so
    // that we can half the memory consumption.
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
    DEFINE_FOR(int, EnumValue)
    DEFINE_FOR(const std::string&, String)

#undef DEFINE_FOR

    // Holds mutable getters for repeated fields.
    // We use it for also for const-getters so that we can reduce the RAM
    // usage.
    std::map<uint32_t, MutableFn<void*>> get_RepeatedRef_impls_;

private:
    // For fields which we shouldn't copy
    // We provide a mutable getter and getter which returns a reference
#define DEFINE_FOR(cc_type, CcType)                                    \
    std::map<uint32_t, MutableFn<cc_type*>> mutable_##CcType##_impls_; \
    std::map<uint32_t, GetterFn<const cc_type&>> get_##CcType##_impls_;

    DEFINE_FOR(Message, Message)
    DEFINE_FOR(std::vector<uint64_t>, RepeatedUInt64)
    DEFINE_FOR(std::vector<int64_t>, RepeatedInt64)
    DEFINE_FOR(std::vector<uint32_t>, RepeatedUInt32)
    DEFINE_FOR(std::vector<int32_t>, RepeatedInt32)
    DEFINE_FOR(std::vector<double>, RepeatedDouble)
    DEFINE_FOR(std::vector<float>, RepeatedFloat)
    DEFINE_FOR(std::vector<bool>, RepeatedBool)
    DEFINE_FOR(std::vector<std::string>, RepeatedString)
#undef DEFINE_FOR

public:
    Reflection() {
    }

    ~Reflection() {
    }

#define DEFINE_FOR(cc_type, CcType)                                           \
    void RegisterSet##CcType(uint32_t tag, const SetterFn<cc_type>& setter) { \
        set_##CcType##_impls_[tag] = setter;                                  \
    }                                                                         \
                                                                              \
    void RegisterGet##CcType(uint32_t tag, const GetterFn<cc_type>& getter) { \
        get_##CcType##_impls_[tag] = getter;                                  \
    }                                                                         \
                                                                              \
    void Set##CcType(Message* message, uint32_t tag, cc_type value) const {   \
        auto it = set_##CcType##_impls_.find(tag);                            \
        assert(it != set_##CcType##_impls_.end());                            \
        it->second(message, value);                                           \
    }                                                                         \
                                                                              \
    cc_type Get##CcType(const Message* message, uint32_t tag) const {         \
        auto it = get_##CcType##_impls_.find(tag);                            \
        assert(it != get_##CcType##_impls_.end());                            \
        return it->second(message);                                           \
    }

    // Define Setter registerers

    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(int, EnumValue)
    DEFINE_FOR(const std::string&, String)

#undef DEFINE_FOR  // Define Registerer

#define DEFINE_FOR(cc_type, CcType)                                          \
    void RegisterGet##CcType(                                                \
            uint32_t tag, const GetterFn<const cc_type&>& getter) {          \
        get_##CcType##_impls_[tag] = getter;                                 \
    }                                                                        \
                                                                             \
    void RegisterMutable##CcType(                                            \
            uint32_t tag, const MutableFn<cc_type*>& mut_getter) {           \
        mutable_##CcType##_impls_[tag] = mut_getter;                         \
    }                                                                        \
                                                                             \
    const cc_type& Get##CcType(const Message* message, uint32_t tag) const { \
        auto it = get_##CcType##_impls_.find(tag);                           \
        assert(it != get_##CcType##_impls_.end());                           \
        return it->second(message);                                          \
    }                                                                        \
                                                                             \
    cc_type* Mutable##CcType(Message* message, uint32_t tag) const {         \
        auto it = mutable_##CcType##_impls_.find(tag);                       \
        assert(it != mutable_##CcType##_impls_.end());                       \
        return it->second(message);                                          \
    }

    // Define Getter registerers
    DEFINE_FOR(Message, Message)

#undef DEFINE_FOR  // Define Registerer

    void RegisterMutableRepeatedRef(
            uint32_t tag, const MutableFn<void*>& getter) {
        get_RepeatedRef_impls_[tag] = getter;
    }

    template <typename T>
    std::vector<T>* MutableRepeatedRef(Message* message, uint32_t tag) const {
        auto it = get_RepeatedRef_impls_.find(tag);
        assert(it != get_RepeatedRef_impls_.end());
        const MutableFn<void*>& getter = it->second;
        void* v_ptr = getter(message);
        return static_cast<std::vector<T>*>(v_ptr);
    }

    template <typename T>
    const std::vector<T>& GetRepeatedRef(
            const Message* message, uint32_t tag) const {
        return *MutableRepeatedRef<T>(const_cast<Message*>(message), tag);
    }

// Define adders for repeated fields
#define DEFINE_FOR(cc_type, CcType)                                         \
    void Add##CcType(Message* message, uint32_t tag, cc_type value) const { \
        MutableRepeatedRef<cc_type>(message, tag)->push_back(value);        \
    }

    // Define adders for primitive repeated fields
    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(int, EnumValue)

#undef DEFINE_FOR  // adder

    void AddString(
            Message* message, uint32_t tag, const std::string& value) const {
        MutableRepeatedRef<std::string>(message, tag)->push_back(value);
    }

    Message* AddMessage(Message* message, uint32_t tag) const {
        auto* messages = MutableRepeatedRef<Message*>(message, tag);
        messages->resize(messages->size() + 1);
        return messages->back();
    }
};

}  // namespace decaproto

#endif
