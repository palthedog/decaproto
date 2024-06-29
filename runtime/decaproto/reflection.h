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

#define DEFINE_FOR(cc_arg_type, CcType) \
    std::map<uint32_t, SetterFn<cc_arg_type>> set_##CcType##_impls_;

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

public:
    Reflection() {
    }

    ~Reflection() {
    }

    // Allows the code generator to register setter/getter for the given field.
    // Internal use only.
#define DEFINE_FOR(cc_arg_type, CcType)                                      \
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

    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(const std::string&, String)

#undef DEFINE_FOR

    // Define setters.
    // Don't try to share a macro to generate both setter/getter and similars.
    // We prefer readability rather than writability.

#define DEFINE_FOR(cc_arg_type, CcType)                                    \
    void Set##CcType(                                                      \
        Message* message, const FieldDescriptor* field, cc_arg_type value) \
        const {                                                            \
        auto it = set_##CcType##_impls_.find(field->GetTag());             \
        assert(it != set_##CcType##_impls_.end());                         \
        it->second(message, value);                                        \
    }

    DEFINE_FOR(uint64_t, UInt64)
    DEFINE_FOR(int64_t, Int64)
    DEFINE_FOR(uint32_t, UInt32)
    DEFINE_FOR(int32_t, Int32)
    DEFINE_FOR(double, Double)
    DEFINE_FOR(float, Float)
    DEFINE_FOR(bool, Bool)
    DEFINE_FOR(const std::string&, String)

#undef DEFINE_FOR

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
