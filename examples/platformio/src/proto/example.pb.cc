#include "example.pb.h"

#include <cassert>
#include "decaproto/reflection_util.h"
#include "decaproto/encoder.h"
#include "decaproto/stream/coded_stream.h"


// A singleton Descriptor for Detail
decaproto::Descriptor* kDetail__Descriptor = nullptr;

const decaproto::Descriptor* Detail::GetDescriptor() const {
    if (kDetail__Descriptor != nullptr) {
        return kDetail__Descriptor;
    }
    kDetail__Descriptor = new decaproto::Descriptor();
    kDetail__Descriptor->RegisterField(decaproto::FieldDescriptor(1, decaproto::FieldType::kDouble));
    kDetail__Descriptor->RegisterField(decaproto::FieldDescriptor(2, decaproto::FieldType::kDouble));
    return kDetail__Descriptor;
}

// A singleton Reflection object for Detail
decaproto::Reflection* kDetail__Reflection = nullptr;

const decaproto::Reflection* Detail::GetReflection() const {
    if (kDetail__Reflection != nullptr) {
        return kDetail__Reflection;
    }
    kDetail__Reflection = new decaproto::Reflection();
    
    // Setter
    kDetail__Reflection->RegisterSetDouble(
        1,
		decaproto::MsgCast(&Detail::set_value_a));
    // Getter
    kDetail__Reflection->RegisterGetDouble(
        1,
		decaproto::MsgCast(&Detail::value_a));
        
    // Setter
    kDetail__Reflection->RegisterSetDouble(
        2,
		decaproto::MsgCast(&Detail::set_value_b));
    // Getter
    kDetail__Reflection->RegisterGetDouble(
        2,
		decaproto::MsgCast(&Detail::value_b));
        return kDetail__Reflection;
}

size_t Detail::ComputeEncodedSize() const {
    size_t size = 0;

		if ( value_a__ != double() ) {
			size += 1;  // tag
			size += 8;
		}
		
		if ( value_b__ != double() ) {
			size += 1;  // tag
			size += 8;
		}
				return size;
}

bool Detail::EncodeImpl(decaproto::CodedOutputStream& stream) const {

					if (value_a__ != double()) {
						stream.WriteTag(1, decaproto::WireType::kI64);
						stream.WriteFixedInt64(decaproto::MemcpyCast<double, uint64_t>(value_a__));
					}
					
					if (value_b__ != double()) {
						stream.WriteTag(2, decaproto::WireType::kI64);
						stream.WriteFixedInt64(decaproto::MemcpyCast<double, uint64_t>(value_b__));
					}
							return true;
}

// A singleton Descriptor for State
decaproto::Descriptor* kState__Descriptor = nullptr;

const decaproto::Descriptor* State::GetDescriptor() const {
    if (kState__Descriptor != nullptr) {
        return kState__Descriptor;
    }
    kState__Descriptor = new decaproto::Descriptor();
    kState__Descriptor->RegisterField(decaproto::FieldDescriptor(1, decaproto::FieldType::kUint32));
    kState__Descriptor->RegisterField(decaproto::FieldDescriptor(2, decaproto::FieldType::kUint32));
    kState__Descriptor->RegisterField(decaproto::FieldDescriptor(3, decaproto::FieldType::kDouble));
    kState__Descriptor->RegisterField(decaproto::FieldDescriptor(4, decaproto::FieldType::kBool));
    kState__Descriptor->RegisterField(decaproto::FieldDescriptor(5, decaproto::FieldType::kMessage));
    return kState__Descriptor;
}

// A singleton Reflection object for State
decaproto::Reflection* kState__Reflection = nullptr;

const decaproto::Reflection* State::GetReflection() const {
    if (kState__Reflection != nullptr) {
        return kState__Reflection;
    }
    kState__Reflection = new decaproto::Reflection();
    
    // Setter
    kState__Reflection->RegisterSetUint32(
        1,
		decaproto::MsgCast(&State::set_timestamp));
    // Getter
    kState__Reflection->RegisterGetUint32(
        1,
		decaproto::MsgCast(&State::timestamp));
        
    // Setter
    kState__Reflection->RegisterSetUint32(
        2,
		decaproto::MsgCast(&State::set_id));
    // Getter
    kState__Reflection->RegisterGetUint32(
        2,
		decaproto::MsgCast(&State::id));
        
    // Setter
    kState__Reflection->RegisterSetDouble(
        3,
		decaproto::MsgCast(&State::set_double_value));
    // Getter
    kState__Reflection->RegisterGetDouble(
        3,
		decaproto::MsgCast(&State::double_value));
        
    // Setter
    kState__Reflection->RegisterSetBool(
        4,
		decaproto::MsgCast(&State::set_bool_value));
    // Getter
    kState__Reflection->RegisterGetBool(
        4,
		decaproto::MsgCast(&State::bool_value));
        
    // Mutable getter for detail
    kState__Reflection->RegisterMutableMessage(
        5,
		decaproto::MsgCast(&State::mutable_detail));
    // Getter for detail
    kState__Reflection->RegisterGetMessage(
        5,
		decaproto::MsgCast(&State::detail));
    // Hazzer for detail
    kState__Reflection->RegisterHasField(
        5,
		decaproto::MsgCast(&State::has_detail));
    return kState__Reflection;
}

size_t State::ComputeEncodedSize() const {
    size_t size = 0;

		if ( timestamp__ != uint32_t() ) {
			size += 1;  // tag
			size += decaproto::ComputeEncodedVarintSize(timestamp__);
		}
		
		if ( id__ != uint32_t() ) {
			size += 1;  // tag
			size += decaproto::ComputeEncodedVarintSize(id__);
		}
		
		if ( double_value__ != double() ) {
			size += 1;  // tag
			size += 8;
		}
		
		if ( bool_value__ != bool() ) {
			size += 1;  // tag
			size += decaproto::ComputeEncodedVarintSize(bool_value__);
		}
		
		if ( has_detail() ) {
			size_t sub_msg_size = detail__->ComputeEncodedSize();
			// tag
			size += 1;
			// LEN
			size += decaproto::ComputeEncodedVarintSize(sub_msg_size);
			// value
			size += sub_msg_size;
		}
				return size;
}

bool State::EncodeImpl(decaproto::CodedOutputStream& stream) const {

					if (timestamp__ != uint32_t()) {
						stream.WriteTag(1, decaproto::WireType::kVarint);
						stream.WriteVarint32(timestamp__);
					}
					
					if (id__ != uint32_t()) {
						stream.WriteTag(2, decaproto::WireType::kVarint);
						stream.WriteVarint32(id__);
					}
					
					if (double_value__ != double()) {
						stream.WriteTag(3, decaproto::WireType::kI64);
						stream.WriteFixedInt64(decaproto::MemcpyCast<double, uint64_t>(double_value__));
					}
					
					if (bool_value__ != bool()) {
						stream.WriteTag(4, decaproto::WireType::kVarint);
						stream.WriteVarint32(bool_value__);
					}
					
					if (has_detail()) {
						size_t sub_msg_size = detail__->ComputeEncodedSize();
						stream.WriteTag(5, decaproto::WireType::kLen);
						stream.WriteVarint32(sub_msg_size);
						detail__->EncodeImpl(stream);
					}
							return true;
}

// A singleton Descriptor for Response
decaproto::Descriptor* kResponse__Descriptor = nullptr;

const decaproto::Descriptor* Response::GetDescriptor() const {
    if (kResponse__Descriptor != nullptr) {
        return kResponse__Descriptor;
    }
    kResponse__Descriptor = new decaproto::Descriptor();
    kResponse__Descriptor->RegisterField(decaproto::FieldDescriptor(1, decaproto::FieldType::kMessage, true));
    return kResponse__Descriptor;
}

// A singleton Reflection object for Response
decaproto::Reflection* kResponse__Reflection = nullptr;

const decaproto::Reflection* Response::GetReflection() const {
    if (kResponse__Reflection != nullptr) {
        return kResponse__Reflection;
    }
    kResponse__Reflection = new decaproto::Reflection();
    
			// Mutable getter for states
			kResponse__Reflection->RegisterGetRepeatedMessage(
				1,
				decaproto::MsgCast(&Response::get_states));
			kResponse__Reflection->RegisterAddRepeatedMessage(
				1,
				decaproto::MsgCast(&Response::add_states));
			kResponse__Reflection->RegisterFieldSize(
				1,
				decaproto::MsgCast(&Response::states_size));
		    return kResponse__Reflection;
}

size_t Response::ComputeEncodedSize() const {
    size_t size = 0;

		for (auto& item : states__) {
			size_t sub_msg_size = item.ComputeEncodedSize();
			// tag
			size += 1;
			// LEN
			size += decaproto::ComputeEncodedVarintSize(sub_msg_size);
			// value
			size += sub_msg_size;
		}
				return size;
}

bool Response::EncodeImpl(decaproto::CodedOutputStream& stream) const {

					for (auto& item : states__) {
						size_t sub_msg_size = item.ComputeEncodedSize();
						stream.WriteTag(1, decaproto::WireType::kLen);
						stream.WriteVarint32(sub_msg_size);
						item.EncodeImpl(stream);
					}
							return true;
}
