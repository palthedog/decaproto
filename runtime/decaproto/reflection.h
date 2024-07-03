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

class Reflection final {
    template <typename T>
    using SetterFn = std::function<void(Message*, T)>;

    template <typename T>
    using GetterFn = std::function<T(const Message*)>;

    template <typename T>
    using MutableFn = std::function<T(Message*)>;

    // TODO: Consider adding/using a single field accessor to implement both so
    // that we can half the memory consumption.
    // key: field number
    // value: accessor function
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
    // std::map<uint32_t, MutableFn<void*>> get_RepeatedRef_impls_;

private:
    template <typename T>
    using SetRepeatedFn = std::function<void(Message*, size_t, T)>;

    template <typename T>
    using GetRepeatedFn = std::function<T(const Message*, size_t)>;

    template <typename T>
    using AddRepeatedFn = std::function<T*(Message*)>;

    using SizeRepeatedFn = std::function<size_t(const Message*)>;

    // For fields which we shouldn't copy
    // We provide a mutable getter and getter which returns a reference
    std::map<uint32_t, SizeRepeatedFn> field_size_impls_;

public:
    void RegisterFieldSize(uint32_t tag, const SizeRepeatedFn& size_fn) {
        field_size_impls_[tag] = size_fn;
    }

    size_t FieldSize(const Message* message, uint32_t tag) const {
        auto it = field_size_impls_.find(tag);
        assert(it != field_size_impls_.end());
        return it->second(message);
    }

    // Registerer for repeated fields

#define DEFINE_FOR(cc_type, CcType)                                            \
private:                                                                       \
    std::map<uint32_t, SetRepeatedFn<cc_type>> set_repeated_##CcType##_impls_; \
    std::map<uint32_t, GetRepeatedFn<cc_type>> get_repeated_##CcType##_impls_; \
    std::map<uint32_t, AddRepeatedFn<cc_type>> add_repeated_##CcType##_impls_; \
                                                                               \
public:                                                                        \
    void RegisterSetRepeated##CcType(                                          \
            uint32_t tag, const SetRepeatedFn<cc_type>& setter) {              \
        set_repeated_##CcType##_impls_[tag] = setter;                          \
    }                                                                          \
    void RegisterGetRepeated##CcType(                                          \
            uint32_t tag, const GetRepeatedFn<cc_type>& getter) {              \
        get_repeated_##CcType##_impls_[tag] = getter;                          \
    }                                                                          \
    void RegisterAddRepeated##CcType(                                          \
            uint32_t tag, const AddRepeatedFn<cc_type>& adder) {               \
        add_repeated_##CcType##_impls_[tag] = adder;                           \
    }                                                                          \
    void SetRepeated##CcType(                                                  \
            Message* message, uint32_t tag, int index, cc_type value) const {  \
        auto it = set_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != set_repeated_##CcType##_impls_.end());                    \
        it->second(message, index, value);                                     \
    }                                                                          \
    cc_type GetRepeated##CcType(                                               \
            const Message* message, uint32_t tag, size_t index) const {        \
        auto it = get_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != get_repeated_##CcType##_impls_.end());                    \
        return it->second(message, index);                                     \
    }                                                                          \
    cc_type* AddRepeated##CcType(Message* message, uint32_t tag) const {       \
        auto it = add_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != add_repeated_##CcType##_impls_.end());                    \
        return it->second(message);                                            \
    }

    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(int, EnumValue)

#undef DEFINE_FOR

#define DEFINE_FOR(cc_type, CcType)                                            \
private:                                                                       \
    std::map<uint32_t, GetRepeatedFn<const cc_type&>>                          \
            get_repeated_##CcType##_impls_;                                    \
    std::map<uint32_t, AddRepeatedFn<cc_type>> add_repeated_##CcType##_impls_; \
    std::map<uint32_t, MutableFn<cc_type*>> mutable_##CcType##_impls_;         \
    std::map<uint32_t, GetterFn<const cc_type&>> getter_##CcType##_impls_;     \
                                                                               \
public:                                                                        \
    void RegisterGetRepeated##CcType(                                          \
            uint32_t tag, const GetRepeatedFn<const cc_type&>& getter) {       \
        get_repeated_##CcType##_impls_[tag] = getter;                          \
    }                                                                          \
    void RegisterAddRepeated##CcType(                                          \
            uint32_t tag, const AddRepeatedFn<cc_type>& adder) {               \
        add_repeated_##CcType##_impls_[tag] = adder;                           \
    }                                                                          \
    void RegisterMutable##CcType(                                              \
            uint32_t tag, const MutableFn<cc_type*>& mut_getter) {             \
        mutable_##CcType##_impls_[tag] = mut_getter;                           \
    }                                                                          \
    void RegisterGet##CcType(                                                  \
            uint32_t tag, const GetterFn<const cc_type&>& getter) {            \
        getter_##CcType##_impls_[tag] = getter;                                \
    }                                                                          \
                                                                               \
    const cc_type& GetRepeated##CcType(                                        \
            const Message* message, uint32_t tag, int index) const {           \
        auto it = get_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != get_repeated_##CcType##_impls_.end());                    \
        return it->second(message, index);                                     \
    }                                                                          \
    cc_type* AddRepeated##CcType(Message* message, uint32_t tag) const {       \
        auto it = add_repeated_##CcType##_impls_.find(tag);                    \
        assert(it != add_repeated_##CcType##_impls_.end());                    \
        return it->second(message);                                            \
    }                                                                          \
    cc_type* Mutable##CcType(Message* message, uint32_t tag) const {           \
        auto it = mutable_##CcType##_impls_.find(tag);                         \
        assert(it != mutable_##CcType##_impls_.end());                         \
        return it->second(message);                                            \
    }                                                                          \
    const cc_type& Get##CcType(const Message* message, uint32_t tag) const {   \
        auto it = getter_##CcType##_impls_.find(tag);                          \
        assert(it != getter_##CcType##_impls_.end());                          \
        return it->second(message);                                            \
    }

    DEFINE_FOR(std::string, String)
    DEFINE_FOR(Message, Message)

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

#undef DEFINE_FOR  // Define Registerer

    /*
        void RegisterMutableRepeatedRef(
                uint32_t tag, const MutableFn<void*>& getter) {
            get_RepeatedRef_impls_[tag] = getter;
        }
        */

    /*
        template <typename T>
        std::vector<T>* MutableRepeatedRef(Message* message, uint32_t tag) const
    { auto it = get_RepeatedRef_impls_.find(tag); assert(it !=
    get_RepeatedRef_impls_.end()); const MutableFn<void*>& getter = it->second;
            void* v_ptr = getter(message);
            return static_cast<std::vector<T>*>(v_ptr);
        }

        template <typename T>
        const std::vector<T>& GetRepeatedRef(
                const Message* message, uint32_t tag) const {
            return *MutableRepeatedRef<T>(const_cast<Message*>(message), tag);
        }
    // Define adders for repeated fields
    #define DEFINE_FOR(cc_type, CcType) \
        void Add##CcType(Message* message, uint32_t tag, cc_type value) const {
    \
            MutableRepeatedRef<cc_type>(message, tag)->push_back(value); \
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
                Message* message, uint32_t tag, const std::string& value) const
    { MutableRepeatedRef<std::string>(message, tag)->push_back(value);
        }

        Message* AddMessage(Message* message, uint32_t tag) const {
            auto* messages = MutableRepeatedRef<Message*>(message, tag);
            messages->resize(messages->size() + 1);
            return messages->back();
        }
        */
};

}  // namespace decaproto

#endif
