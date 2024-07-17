package main

import (
	"fmt"
	"os"

	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
)

func printEncoder(m *descriptor.DescriptorProto, ctx *Context, msg_printer *MessagePrinter) {
	// Declaration
	msg_printer.publics += "    bool EncodeImpl(decaproto::CodedOutputStream& stream) const override;\n"

	// Definition
	var src string = ""
	src += "\n"
	src += "bool " + msg_printer.full_name + "::EncodeImpl(decaproto::CodedOutputStream& stream) const {\n"
	for _, f := range m.GetField() {
		type_name_info := getTypeNameInfo(f)
		args := map[string]string{
			"name":        f.GetName(),
			"field_num":   fmt.Sprintf("%d", f.GetNumber()),
			"holder_name": holderName(f),
			"cc_type":     type_name_info.cc_type,
		}
		if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
			// Non-repeated field
			switch f.GetType() {
			case descriptor.FieldDescriptorProto_TYPE_INT32,
				descriptor.FieldDescriptorProto_TYPE_UINT32,
				descriptor.FieldDescriptorProto_TYPE_ENUM,
				descriptor.FieldDescriptorProto_TYPE_BOOL:
				src += print("rep_varint32_enc", `
					for (auto item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteVarint32(item);
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_INT64,
				descriptor.FieldDescriptorProto_TYPE_UINT64:
				src += print("rep_varint64_enc", `
					for (auto item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteVarint64(item);
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED32,
				descriptor.FieldDescriptorProto_TYPE_SFIXED32:
				src += print("rep_fixed32_enc", `
					for (auto item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI32);
						stream.WriteFixedInt32(item);
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_FLOAT:
				src += print("rep_float_enc", `
					for (float item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI32);
						stream.WriteFixedInt32(decaproto::MemcpyCast<float, uint32_t>(item));
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED64,
				descriptor.FieldDescriptorProto_TYPE_SFIXED64:
				src += print("rep_float_enc", `
					for (auto item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI64);
						stream.WriteFixedInt64(item);
					}
					`, args)

			case descriptor.FieldDescriptorProto_TYPE_DOUBLE:
				src += print("rep_double_enc", `
					for (double item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI64);
						stream.WriteFixedInt64(decaproto::MemcpyCast<double, uint64_t>(item));
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_SINT32:
				src += print("rep_sint32_enc", `
					for (auto item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteSignedVarint32(item);
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_SINT64:
				src += print("rep_sint64_enc", `
					for (auto item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteSignedVarint64(item);
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_STRING:
				src += print("rep_str_enc", `
					for (auto& item : {{.holder_name}}) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kLen);
						stream.WriteVarint32(item.size());
						stream.WriteString(item);
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_MESSAGE:
				src += print("rep_sub_msg_enc", `
					for (auto& item : {{.holder_name}}) {
						size_t sub_msg_size = item.ComputeEncodedSize();
						stream.WriteTag({{.field_num}}, decaproto::WireType::kLen);
						stream.WriteVarint32(sub_msg_size);
						item.EncodeImpl(stream);
					}
					`, args)
			default:
				fmt.Fprintf(os.Stderr, "%s %s repeated field is not supported yet\n", f.GetTypeName(), f.GetName())
				os.Exit(1)
			}
		} else {
			switch f.GetType() {
			case descriptor.FieldDescriptorProto_TYPE_INT32,
				descriptor.FieldDescriptorProto_TYPE_UINT32,
				descriptor.FieldDescriptorProto_TYPE_ENUM,
				descriptor.FieldDescriptorProto_TYPE_BOOL:
				src += print("varint32_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteVarint32({{.holder_name}});
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_INT64,
				descriptor.FieldDescriptorProto_TYPE_UINT64:
				src += print("varint64_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteVarint64({{.holder_name}});
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED32,
				descriptor.FieldDescriptorProto_TYPE_SFIXED32:
				src += print("fixed32_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI32);
						stream.WriteFixedInt32({{.holder_name}});
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_FLOAT:
				src += print("float_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI32);
						stream.WriteFixedInt32(decaproto::MemcpyCast<float, uint32_t>({{.holder_name}}));
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED64,
				descriptor.FieldDescriptorProto_TYPE_SFIXED64:
				src += print("float_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI64);
						stream.WriteFixedInt64({{.holder_name}});
					}
					`, args)

			case descriptor.FieldDescriptorProto_TYPE_DOUBLE:
				src += print("double_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kI64);
						stream.WriteFixedInt64(decaproto::MemcpyCast<double, uint64_t>({{.holder_name}}));
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_SINT32:
				src += print("sint32_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteSignedVarint32({{.holder_name}});
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_SINT64:
				src += print("sint64_enc", `
					if ({{.holder_name}} != {{.cc_type}}()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kVarint);
						stream.WriteSignedVarint64({{.holder_name}});
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_STRING:
				src += print("str_enc", `
					if (!{{.holder_name}}.empty()) {
						stream.WriteTag({{.field_num}}, decaproto::WireType::kLen);
						stream.WriteVarint32({{.holder_name}}.size());
						stream.WriteString({{.holder_name}});
					}
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_MESSAGE:
				src += print("rep_str_enc", `
					if (has_{{.name}}()) {
						size_t sub_msg_size = {{.holder_name}}->ComputeEncodedSize();
						stream.WriteTag({{.field_num}}, decaproto::WireType::kLen);
						stream.WriteVarint32(sub_msg_size);
						{{.holder_name}}->EncodeImpl(stream);
					}
					`, args)
			default:
				fmt.Fprintf(os.Stderr, "%s %s field is not supported yet\n", f.GetTypeName(), f.GetName())
				os.Exit(1)
			}
		}
	}
	src += "		return true;\n"
	src += "}\n"
	ctx.printer.source_content += src
}

func printComputeEncodedSize(m *descriptor.DescriptorProto, ctx *Context, msg_printer *MessagePrinter) {
	// Declaration
	msg_printer.publics += "    size_t ComputeEncodedSize() const override;\n"

	// Definition
	var src string = ""
	src += "\n"
	src += "size_t " + msg_printer.full_name + "::ComputeEncodedSize() const {\n"
	src += "    size_t size = 0;\n"
	for _, f := range m.GetField() {
		args := map[string]string{
			"name":        f.GetName(),
			"holder_name": holderName(f),
			"cc_type":     getTypeNameInfo(f).cc_type,
		}
		if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
			// Non-repeated field
			switch f.GetType() {
			case descriptor.FieldDescriptorProto_TYPE_INT32,
				descriptor.FieldDescriptorProto_TYPE_INT64,
				descriptor.FieldDescriptorProto_TYPE_UINT32,
				descriptor.FieldDescriptorProto_TYPE_UINT64,
				descriptor.FieldDescriptorProto_TYPE_ENUM,
				descriptor.FieldDescriptorProto_TYPE_BOOL:
				src += print("rep_varint_size", `
		for (auto& item : {{.holder_name}}) {
			size += 1;  // tag
			size += decaproto::ComputeEncodedVarintSize(item);
		}
		`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED32,
				descriptor.FieldDescriptorProto_TYPE_SFIXED32,
				descriptor.FieldDescriptorProto_TYPE_FLOAT:
				src += print("rep_fixed32_size", `
		size += (1 + 4) * {{.holder_name}}.size();
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED64,
				descriptor.FieldDescriptorProto_TYPE_SFIXED64,
				descriptor.FieldDescriptorProto_TYPE_DOUBLE:
				src += print("rep_fixed32_size", `
	    size += (1 + 8) * {{.holder_name}}.size();
					`, args)
			case descriptor.FieldDescriptorProto_TYPE_SINT32:
			case descriptor.FieldDescriptorProto_TYPE_SINT64:
				src += print("sint_size", `
		for (auto item : {{.holder_name}}) {
			int64_t zigzag = decaproto::CodedOutputStream::EncodeZigZag(item);
			size += 1;  // tag
			size += decaproto::ComputeEncodedVarintSize(zigzag);
		}
		`, args)

			case descriptor.FieldDescriptorProto_TYPE_STRING:
				src += print("rep_fixed64_size", `
		for (auto& item : {{.holder_name}}) {
			// tag
			size += 1;
			// LEN
			size += decaproto::ComputeEncodedVarintSize(item.size());
			// value
			size += item.size();
		}
		`, args)
			case descriptor.FieldDescriptorProto_TYPE_MESSAGE:
				src += print("rep_msg_size", `
		for (auto& item : {{.holder_name}}) {
			size_t sub_msg_size = item.ComputeEncodedSize();
			// tag
			size += 1;
			// LEN
			size += decaproto::ComputeEncodedVarintSize(sub_msg_size);
			// value
			size += sub_msg_size;
		}
		`, args)
			default:
				fmt.Fprintf(os.Stderr, "%s %s repeated field is not supported yet\n", f.GetTypeName(), f.GetName())
				os.Exit(1)
			}
		} else {
			// Non-repeated field
			switch f.GetType() {
			case descriptor.FieldDescriptorProto_TYPE_INT32,
				descriptor.FieldDescriptorProto_TYPE_INT64,
				descriptor.FieldDescriptorProto_TYPE_UINT32,
				descriptor.FieldDescriptorProto_TYPE_UINT64,
				descriptor.FieldDescriptorProto_TYPE_ENUM,
				descriptor.FieldDescriptorProto_TYPE_BOOL:
				src += print("varint_size", `
		if ( {{.holder_name}} != {{.cc_type}}() ) {
			size += 1;  // tag
			size += decaproto::ComputeEncodedVarintSize({{.holder_name}});
		}
		`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED32,
				descriptor.FieldDescriptorProto_TYPE_SFIXED32,
				descriptor.FieldDescriptorProto_TYPE_FLOAT:
				src += print("fixed32_size", `
		if ( {{.holder_name}} != {{.cc_type}}() ) {
			size += 1;  // tag
			size += 4;
		}
		`, args)
			case descriptor.FieldDescriptorProto_TYPE_FIXED64,
				descriptor.FieldDescriptorProto_TYPE_SFIXED64,
				descriptor.FieldDescriptorProto_TYPE_DOUBLE:
				src += print("fixed64_size", `
		if ( {{.holder_name}} != {{.cc_type}}() ) {
			size += 1;  // tag
			size += 8;
		}
		`, args)
			case descriptor.FieldDescriptorProto_TYPE_SINT32:
			case descriptor.FieldDescriptorProto_TYPE_SINT64:
				src += print("sint_size", `
		if ( {{.holder_name}} != {{.cc_type}}() ) {
			int64_t zigzag = decaproto::CodedOutputStream::EncodeZigZag({{.holder_name}});
			size += 1;  // tag
			size += decaproto::ComputeEncodedVarintSize(zigzag);
		}
`, args)
			case descriptor.FieldDescriptorProto_TYPE_STRING:
				src += print("str_size", `
		if ( !{{.holder_name}}.empty() ) {
			// tag
			size += 1;
			// LEN
			size += decaproto::ComputeEncodedVarintSize({{.holder_name}}.size());
			// value
			size += {{.holder_name}}.size();
		}
		`, args)
			case descriptor.FieldDescriptorProto_TYPE_MESSAGE:
				src += print("msg_size", `
		if ( has_{{.name}}() ) {
			size_t sub_msg_size = {{.holder_name}}->ComputeEncodedSize();
			// tag
			size += 1;
			// LEN
			size += decaproto::ComputeEncodedVarintSize(sub_msg_size);
			// value
			size += sub_msg_size;
		}
		`, args)
			default:
				fmt.Fprintf(os.Stderr, "%s %s field is not supported yet\n", f.GetTypeName(), f.GetName())
				os.Exit(1)
			}
		}
	}
	src += "		return size;\n"
	src += "}\n"
	ctx.printer.source_content += src
}
