#ifndef DECAPROTO_REFLECTION_H
#define DECAPROTO_REFLECTION_H

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>

#include "decaproto/descriptor.h"

namespace decaproto {

class Message;

// Reflection class provides ways to access to fields of a message without
// knowing the concrete type of the message.
//
// Library users don't need to use this class for most cases.
// However, it's useful if you want to develop a utility library working with
// deca proto (e.g. serialization library).
//
// For example, we have a Message with a uint32 field with tag 1.
// message SampleProto {
//   uint32 num = 1;
// }
//
// We noramally access the field like this:
//   SampleProto sample;
//   sample.set_num(10);
//   std::cout << sample.num() << std::endl;
// But with Reflection, we can access the field like this:
//   Message* sample;  // don't know the concrete type here
//   Reflection* reflection = sample->GetReflection();
//   const int kFieldNumber = 1;
//   reflection->SetUint32(sample, kFieldNumber, 10);
//   std::cout << reflection->GetUint32(sample, kFieldNumber) << std::endl;
class Reflection final {
    template <typename T>
    using SetterFn = std::function<void(Message*, T)>;

    template <typename T>
    using GetterFn = std::function<T(const Message*)>;

    template <typename T>
    using MutableFn = std::function<T(Message*)>;

    template <typename T>
    using SetRepeatedFn = std::function<void(Message*, size_t, T)>;

    template <typename T>
    using GetRepeatedFn = std::function<T(const Message*, size_t)>;

    template <typename T>
    using AddRepeatedFn = std::function<T*(Message*)>;

    using SizeRepeatedFn = std::function<size_t(const Message*)>;

    using HasFn = std::function<bool(const Message*)>;

    // Define accessors and register functions for primitive types

#define DEFINE_FOR(cc_type, CcType)                                            \
private:                                                                       \
    std::map<uint32_t, SetterFn<cc_type>> set_##CcType##_impls_;               \
    std::map<uint32_t, GetterFn<cc_type>> get_##CcType##_impls_;               \
    std::map<uint32_t, SetRepeatedFn<cc_type>> set_repeated_##CcType##_impls_; \
    std::map<uint32_t, GetRepeatedFn<cc_type>> get_repeated_##CcType##_impls_; \
    std::map<uint32_t, AddRepeatedFn<cc_type>> add_repeated_##CcType##_impls_; \
                                                                               \
public:                                                                        \
    void RegisterSet##CcType(uint32_t tag, const SetterFn<cc_type>& setter) {  \
        set_##CcType##_impls_[tag] = setter;                                   \
    }                                                                          \
                                                                               \
    void RegisterGet##CcType(uint32_t tag, const GetterFn<cc_type>& getter) {  \
        get_##CcType##_impls_[tag] = getter;                                   \
    }                                                                          \
                                                                               \
    void Set##CcType(Message* message, uint32_t tag, cc_type value) const {    \
        auto it = set_##CcType##_impls_.find(tag);                             \
        assert(it != set_##CcType##_impls_.end());                             \
        it->second(message, value);                                            \
    }                                                                          \
                                                                               \
    cc_type Get##CcType(const Message* message, uint32_t tag) const {          \
        auto it = get_##CcType##_impls_.find(tag);                             \
        assert(it != get_##CcType##_impls_.end());                             \
        return it->second(message);                                            \
    }                                                                          \
                                                                               \
    void SetRepeated##CcType(                                                  \
            Message* message, uint32_t tag, int index, cc_type value) const {  \
        auto it = set_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != set_repeated_##CcType##_impls_.end());                    \
        it->second(message, index, value);                                     \
    }                                                                          \
                                                                               \
    cc_type GetRepeated##CcType(                                               \
            const Message* message, uint32_t tag, size_t index) const {        \
        auto it = get_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != get_repeated_##CcType##_impls_.end());                    \
        return it->second(message, index);                                     \
    }                                                                          \
                                                                               \
    cc_type* AddRepeated##CcType(Message* message, uint32_t tag) const {       \
        auto it = add_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != add_repeated_##CcType##_impls_.end());                    \
        return it->second(message);                                            \
    }                                                                          \
                                                                               \
    void RegisterSetRepeated##CcType(                                          \
            uint32_t tag, const SetRepeatedFn<cc_type>& setter) {              \
        set_repeated_##CcType##_impls_[tag] = setter;                          \
    }                                                                          \
                                                                               \
    void RegisterGetRepeated##CcType(                                          \
            uint32_t tag, const GetRepeatedFn<cc_type>& getter) {              \
        get_repeated_##CcType##_impls_[tag] = getter;                          \
    }                                                                          \
                                                                               \
    void RegisterAddRepeated##CcType(                                          \
            uint32_t tag, const AddRepeatedFn<cc_type>& adder) {               \
        add_repeated_##CcType##_impls_[tag] = adder;                           \
    }

    DEFINE_FOR(uint64_t, Uint64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(int64_t, Sint64)
    DEFINE_FOR(uint64_t, Fixed64)
    DEFINE_FOR(int64_t, Sfixed64)
    DEFINE_FOR(uint32_t, Uint32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(int32_t, Sint32)
    DEFINE_FOR(uint32_t, Fixed32)
    DEFINE_FOR(int32_t, Sfixed32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(int, EnumValue)

#undef DEFINE_FOR

    // Define accessors and register functions for string and message types

#define DEFINE_FOR(cc_type, CcType)                                            \
private:                                                                       \
    std::map<uint32_t, GetRepeatedFn<const cc_type&>>                          \
            get_repeated_##CcType##_impls_;                                    \
    std::map<uint32_t, AddRepeatedFn<cc_type>> add_repeated_##CcType##_impls_; \
    std::map<uint32_t, MutableFn<cc_type*>> mutable_##CcType##_impls_;         \
    std::map<uint32_t, GetterFn<const cc_type&>> getter_##CcType##_impls_;     \
                                                                               \
public:                                                                        \
    const cc_type& GetRepeated##CcType(                                        \
            const Message* message, uint32_t tag, int index) const {           \
        auto it = get_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != get_repeated_##CcType##_impls_.end());                    \
        return it->second(message, index);                                     \
    }                                                                          \
                                                                               \
    cc_type* AddRepeated##CcType(Message* message, uint32_t tag) const {       \
        auto it = add_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != add_repeated_##CcType##_impls_.end());                    \
        return it->second(message);                                            \
    }                                                                          \
                                                                               \
    cc_type* Mutable##CcType(Message* message, uint32_t tag) const {           \
        auto it = mutable_##CcType##_impls_.find(tag);                         \
        assert(it != mutable_##CcType##_impls_.end());                         \
        return it->second(message);                                            \
    }                                                                          \
                                                                               \
    const cc_type& Get##CcType(const Message* message, uint32_t tag) const {   \
        auto it = getter_##CcType##_impls_.find(tag);                          \
        assert(it != getter_##CcType##_impls_.end());                          \
        return it->second(message);                                            \
    }                                                                          \
                                                                               \
    void RegisterGetRepeated##CcType(                                          \
            uint32_t tag, const GetRepeatedFn<const cc_type&>& getter) {       \
        get_repeated_##CcType##_impls_[tag] = getter;                          \
    }                                                                          \
                                                                               \
    void RegisterAddRepeated##CcType(                                          \
            uint32_t tag, const AddRepeatedFn<cc_type>& adder) {               \
        add_repeated_##CcType##_impls_[tag] = adder;                           \
    }                                                                          \
                                                                               \
    void RegisterMutable##CcType(                                              \
            uint32_t tag, const MutableFn<cc_type*>& mut_getter) {             \
        mutable_##CcType##_impls_[tag] = mut_getter;                           \
    }                                                                          \
                                                                               \
    void RegisterGet##CcType(                                                  \
            uint32_t tag, const GetterFn<const cc_type&>& getter) {            \
        getter_##CcType##_impls_[tag] = getter;                                \
    }

    DEFINE_FOR(std::string, String)
    DEFINE_FOR(Message, Message)

#undef DEFINE_FOR

private:
    // For fields which we shouldn't copy
    // We provide a mutable getter and getter which returns a reference
    std::map<uint32_t, SizeRepeatedFn> field_size_impls_;
    std::map<uint32_t, HasFn> has_field_impls_;

public:
    size_t FieldSize(const Message* message, uint32_t tag) const {
        auto it = field_size_impls_.find(tag);
        assert(it != field_size_impls_.end());
        return it->second(message);
    }

    void RegisterFieldSize(uint32_t tag, const SizeRepeatedFn& size_fn) {
        field_size_impls_[tag] = size_fn;
    }

    bool HasField(const Message* message, uint32_t tag) const {
        auto it = has_field_impls_.find(tag);
        std::cerr << "tag: " << tag << std::endl;
        assert(it != has_field_impls_.end());
        return it->second(message);
    }

    void RegisterHasField(uint32_t tag, const HasFn& has_fn) {
        has_field_impls_[tag] = has_fn;
    }
};

}  // namespace decaproto

#endif
