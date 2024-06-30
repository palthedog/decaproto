#ifndef DECAPROTO_REFLECTION_UTIL_H
#define DECAPROTO_REFLECTION_UTIL_H

#include <functional>

#include "decaproto/message.h"

namespace decaproto {

// Casts a method pointer to a functin object.
// It also allows the caller to use the base Message pointer.
template <typename Ret, typename SubMsg, typename... Args>
static std::function<Ret(Message*, Args...)> MsgCast(
        Ret (SubMsg::*method_ptr)(Args...)) {
    // We must specify the return type `-> Ret` to make sure that the
    // lambda function returns method_ptr's return type AS IS.
    // Like, even if `Ret = const vector<int>&`, the compiler would
    // drop reference "&" from the return type.
    return [=](Message* base_message, Args... args) -> Ret {
        SubMsg* message = static_cast<SubMsg*>(base_message);
        return (message->*method_ptr)(args...);
    };
}

// Casts a method pointer to a functin object.
// It also allows the caller to use the base Message pointer.
// This variant is for const methods
template <typename Ret, typename SubMsg, typename... Args>
static std::function<Ret(const Message*, Args...)> MsgCast(
        Ret (SubMsg::*method_ptr)(Args...) const) {
    // We must specify the return type `-> Ret` to make sure that the
    // lambda function returns method_ptr's return type AS IS.
    // Like, even if `Ret = const vector<int>&`, the compiler would
    // drop reference "&" from the return type.
    return [=](const Message* base_message, Args... args) -> Ret {
        const SubMsg* message = static_cast<const SubMsg*>(base_message);
        return (message->*method_ptr)(args...);
    };
}

// Casts a method pointer to a function object.
// It also casts between EnumType and int for the argument.
template <typename FieldEnumType, typename SubMsg>
static std::function<void(Message*, int)> CastForSetEnumValue(
        void (SubMsg::*method_ptr)(FieldEnumType)) {
    // SetEnumValue takes an int.
    // We need to cast the int value into the EnumType
    return [=](Message* base_message, int arg) {
        SubMsg* message = static_cast<SubMsg*>(base_message);
        return (message->*method_ptr)(static_cast<FieldEnumType>(arg));
    };
}

// Casts a method pointer to a function object.
// It also casts between EnumType and int for the return value.
template <typename FieldEnumType, typename SubMsg>
static std::function<int(const Message*)> CastForGetEnumValue(
        FieldEnumType (SubMsg::*method_ptr)() const) {
    // Getters for Enum fields returns Enum type.
    // We need to cast the EnumType into int for GetEnumValue.
    return [=](const Message* base_message) -> int {
        const SubMsg* message = static_cast<const SubMsg*>(base_message);
        return static_cast<int>((message->*method_ptr)());
    };
}

}  // namespace decaproto

#endif  // DECAPROTO_REFLECTION_UTIL_H
