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

#define DECL_ACCESSOR_IMPL(cc_arg_type, CcType) \
    std::map<uint32_t, SetterFn<cc_arg_type>> set_##CcType##_impls_;

    DECL_ACCESSOR_IMPL(uint32_t, UInt32)
    DECL_ACCESSOR_IMPL(int32_t, Int32)
    DECL_ACCESSOR_IMPL(const std::string&, String)

#undef DECL_ACCESSOR_IMPL

public:
    Reflection() {
    }
    ~Reflection() {
    }

    // Allows the code generator to register setter/getter for the given field.
    // Internal use only.
#define DEF_FIELD_REGISTER(cc_arg_type, CcType)                              \
    void Register##CcType##Field(                                            \
        const FieldDescriptor& field, const SetterFn<cc_arg_type>& setter) { \
        uint32_t tag = field.GetTag();                                       \
        set_##CcType##_impls_[tag] = setter;                                 \
    }

    // This is the expanded version of the above macro for reference.
    // DEF_FIELD_REGISTER(uint32_t, uint32, UInt32)
    /*
    void RegisterUInt32Field(
        const FieldDescriptor& field, const SetterFn<uint32_t>& setter) {
        uint32_t tag = field.GetTag();
        set_uint32_impls_[tag] = setter;
    }
    */

    DEF_FIELD_REGISTER(uint32_t, UInt32)
    DEF_FIELD_REGISTER(int32_t, Int32)
    DEF_FIELD_REGISTER(const std::string&, String)

#undef DEF_FIELD_REGISTER

#define DEF_SETTER(cc_arg_type, CcType)                                    \
    void Set##CcType(                                                      \
        Message* message, const FieldDescriptor* field, cc_arg_type value) \
        const {                                                            \
        auto it = set_##CcType##_impls_.find(field->GetTag());             \
        assert(it != set_##CcType##_impls_.end());                         \
        it->second(message, value);                                        \
    }

    DEF_SETTER(uint32_t, UInt32)
    DEF_SETTER(int32_t, Int32)
    DEF_SETTER(const std::string&, String)

#undef DEF_SETTER

    /*
        void SetUInt32(
            Message* message, const FieldDescriptor* field, uint32_t value)
       const { auto it = set_uint32_impls_.find(field->GetTag()); assert(it !=
       set_uint32_impls_.end()); it->second(message, value);
        }
    */
    /*
        virtual void SetInt32(
            Message* message, const FieldDescriptor* field, int32_t value)
       const = 0; virtual void SetInt64( Message* message, const
       FieldDescriptor* field, int64_t value) const = 0;
    */
    /*
     void SetUInt32(
         Message* message, const FieldDescriptor* field, uint32_t value) const {
         auto it = set_uint32_impls_.find(field->GetTag());
         assert(it != set_uint32_impls_.end());
         it->second(message, value);
     }
     */
    /*
    virtual void SetUInt64(
    Message* message, const FieldDescriptor* field, uint64_t value) const =
    0; virtual void SetFloat( Message* message, const FieldDescriptor* field,
    float value) const = 0; virtual void SetDouble( Message* message, const
    FieldDescriptor* field, double value) const = 0; virtual void SetBool(
    Message* message, const FieldDescriptor* field, bool value) const = 0;
    */

    /*
        void SetString(
            Message* message,
            const FieldDescriptor* field,
            const std::string& value) const {
            auto it = set_string_impls_.find(field->GetTag());
            assert(it != set_string_impls_.end());
            it->second(message, value);
        }
        */
    /*
    virtual void SetEnum(
    Message* message, const FieldDescriptor* field, int value) const = 0;

    virtual int32_t GetInt32(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual int64_t GetInt64(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual uint32_t GetUInt32(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual uint64_t GetUInt64(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual float GetFloat(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual double GetDouble(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual bool GetBool(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual const std::string& GetString(
    const Message* message, const FieldDescriptor* field) const const = 0;
    virtual int GetEnum(
    const Message* message, const FieldDescriptor* field) const const = 0;
    */
};  // namespace decaproto

}  // namespace decaproto

#endif
