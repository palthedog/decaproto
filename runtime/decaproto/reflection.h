#ifndef DECAPROTO_REFLECTION_H
#define DECAPROTO_REFLECTION_H

#include <string>

#include "decaproto/descriptor.h"

namespace decaproto {

class Message;

class Reflection {
public:
    Reflection() {
    }
    virtual ~Reflection() {
    }

    /*
        virtual void SetInt32(
            Message* message, const FieldDescriptor* field, int32_t value) const
       = 0; virtual void SetInt64( Message* message, const FieldDescriptor*
       field, int64_t value) const = 0;
    */
    virtual void SetUInt32(
        Message* message,
        const FieldDescriptor* field,
        uint32_t value) const = 0;
    /*
virtual void SetUInt64(
    Message* message, const FieldDescriptor* field, uint64_t value) const = 0;
virtual void SetFloat(
    Message* message, const FieldDescriptor* field, float value) const = 0;
virtual void SetDouble(
    Message* message, const FieldDescriptor* field, double value) const = 0;
virtual void SetBool(
    Message* message, const FieldDescriptor* field, bool value) const = 0;
    */

    virtual void SetString(
        Message* message,
        const FieldDescriptor* field,
        const std::string& value) const = 0;
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
};

}  // namespace decaproto

#endif
