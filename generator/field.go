package main

import (
	"log"
	"strings"

	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
)

type TypeNameInfo struct {
	// Type names...

	// in .proto files
	// e.g. uint64, int32, string
	proto_name string

	// in decaproto FieldType enum
	// e.g. FieldType::kUInt64, FieldType::kInt32, FieldType::kString
	deca_enum_name string

	// in native C++ codes w/o any qualifiers
	// e.g. uint64_t, int32_t, std::string
	cc_raw_type string

	// in native C++ codes. when used as a return value for getters
	// e.g. uint64_t, int32_t, std::string
	cc_ret_type string

	// in native C++ codes. when used as a const value
	// e.g. uint64_t, int32_t, const std::string&
	cc_arg_type string

	// for camel case C++ type
	// e.g. UInt64, Int32, String
	cc_camel_name string
}

func NewPrimitiveTypeNameInfo(proto_name, deca_enum_name, cc_raw_type, cc_camel_name string) TypeNameInfo {
	return TypeNameInfo{
		proto_name:     proto_name,
		deca_enum_name: "decaproto::FieldType::" + deca_enum_name,
		cc_raw_type:    cc_raw_type,
		cc_ret_type:    cc_raw_type,
		cc_arg_type:    cc_raw_type,
		cc_camel_name:  cc_camel_name,
	}
}

func NewObjectTypeNameInfo(proto_name, deca_enum_name, cc_raw_type, cc_camel_name string) TypeNameInfo {
	return TypeNameInfo{
		proto_name:     proto_name,
		deca_enum_name: "decaproto::FieldType::" + deca_enum_name,
		cc_raw_type:    cc_raw_type,
		cc_ret_type:    "const " + cc_raw_type + "&",
		cc_arg_type:    "const " + cc_raw_type + "&",
		cc_camel_name:  cc_camel_name,
	}
}

func NewGeneratedTypeNameInfo(deca_enum_name, cc_camel_name string) TypeNameInfo {
	n_a := "NOT_AVAILABLE_YET_FILL_ME"
	return TypeNameInfo{
		proto_name:     n_a,
		deca_enum_name: "decaproto::FieldType::" + deca_enum_name,
		cc_raw_type:    n_a,
		cc_ret_type:    n_a,
		cc_arg_type:    n_a,
		cc_camel_name:  cc_camel_name,
	}
}

func getTypeNameInfoBase(f *descriptor.FieldDescriptorProto) TypeNameInfo {
	switch f.GetType() {
	case descriptor.FieldDescriptorProto_TYPE_UINT64:
		return NewPrimitiveTypeNameInfo("uint64", "kUInt64", "uint64_t", "UInt64")
	case descriptor.FieldDescriptorProto_TYPE_INT64:
		return NewPrimitiveTypeNameInfo("int64", "kInt64", "int64_t", "Int64")
	case descriptor.FieldDescriptorProto_TYPE_FIXED64:
		return NewPrimitiveTypeNameInfo("fixed64", "kFixed64", "uint64_t", "UInt64")
	case descriptor.FieldDescriptorProto_TYPE_UINT32:
		return NewPrimitiveTypeNameInfo("uint32", "kUInt32", "uint32_t", "UInt32")
	case descriptor.FieldDescriptorProto_TYPE_INT32:
		return NewPrimitiveTypeNameInfo("int32", "kInt32", "int32_t", "Int32")
	case descriptor.FieldDescriptorProto_TYPE_FIXED32:
		return NewPrimitiveTypeNameInfo("fixed32", "kFixed32", "uint32_t", "UInt32")
	case descriptor.FieldDescriptorProto_TYPE_DOUBLE:
		return NewPrimitiveTypeNameInfo("double", "kDouble", "double", "Double")
	case descriptor.FieldDescriptorProto_TYPE_FLOAT:
		return NewPrimitiveTypeNameInfo("float", "kFloat", "float", "Float")
	case descriptor.FieldDescriptorProto_TYPE_BOOL:
		return NewPrimitiveTypeNameInfo("bool", "kBool", "bool", "Bool")
	case descriptor.FieldDescriptorProto_TYPE_STRING:
		return NewObjectTypeNameInfo("string", "kString", "std::string", "String")
	case descriptor.FieldDescriptorProto_TYPE_MESSAGE:
		return NewGeneratedTypeNameInfo("kMessage", "Message")
	// We use "EnumValue" because cc_camel_name is used only for the getter/setter in Reflection
	// and our Reflection interface provides only enum accessors via EnumValue(i.e. int)
	case descriptor.FieldDescriptorProto_TYPE_ENUM:
		return NewGeneratedTypeNameInfo("kEnum", "EnumValue")
	}

	log.Fatal("Unsupported field type to get TypeNameInfo", f.GetType(), f.GetName())
	return TypeNameInfo{}
}

func getTypeNameInfo(f *descriptor.FieldDescriptorProto) TypeNameInfo {
	info := getTypeNameInfoBase(f)
	if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
		// .OtherMessage
		// .OuterMessage.NestedMessage
		proto_type_name := f.GetTypeName()
		// ::OuterMessage::NestedMessage
		cc_raw_type := strings.Join(strings.Split(proto_type_name, "."), "::")
		// Access Message objects via const reference
		info.proto_name = proto_type_name
		info.cc_raw_type = cc_raw_type
		info.cc_arg_type = "const " + cc_raw_type + "&"
		info.cc_ret_type = "const " + cc_raw_type + "&"
	} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
		// .MyEnum
		// .OuterMessage.NestedEnum
		proto_type_name := f.GetTypeName()
		// ::OuterMessage::NestedEnum
		cc_raw_type := strings.Join(strings.Split(proto_type_name, "."), "::")
		// Enum is copyable
		info.proto_name = proto_type_name
		info.cc_raw_type = cc_raw_type
		info.cc_arg_type = cc_raw_type
		info.cc_ret_type = cc_raw_type
	}

	if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
		info.cc_raw_type = "std::vector<" + info.cc_raw_type + ">"
		info.cc_ret_type = "const std::vector<" + info.cc_ret_type + ">&"
		info.cc_arg_type = "const std::vector<" + info.cc_arg_type + ">&"
		info.cc_camel_name = "Repeated" + info.cc_camel_name
	}

	return info
}

func isPrimitiveType(f *descriptor.FieldDescriptorProto) bool {
	switch f.GetType() {
	case descriptor.FieldDescriptorProto_TYPE_UINT64,
		descriptor.FieldDescriptorProto_TYPE_INT64,
		descriptor.FieldDescriptorProto_TYPE_FIXED64,
		descriptor.FieldDescriptorProto_TYPE_UINT32,
		descriptor.FieldDescriptorProto_TYPE_INT32,
		descriptor.FieldDescriptorProto_TYPE_FIXED32,
		descriptor.FieldDescriptorProto_TYPE_DOUBLE,
		descriptor.FieldDescriptorProto_TYPE_FLOAT,
		descriptor.FieldDescriptorProto_TYPE_BOOL:
		return true
	}
	return false
}

func processField(ctx *Context, msg_printer *MessagePrinter, f *descriptor.FieldDescriptorProto) {
	type_name_info := getTypeNameInfo(f)
	// TODO: Replace it with type_name_info so that we can share more logic across
	// different types of fields
	if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
		// repeated
		if isPrimitiveType(f) {
			// primitive types
			addRepeatedPrimitiveField(f, &type_name_info, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// string
			addRepeatedPrimitiveField(f, &type_name_info, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// enum
			addRepeatedPrimitiveField(f, &type_name_info, msg_printer)
		} else {
			log.Fatal("Unsupported repeated field type", f.GetType(), f.GetName())
		}
	} else if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_OPTIONAL {
		// optional
		if isPrimitiveType(f) || f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// primitive types | enum
			addPrimitiveField(f, &type_name_info, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// string
			addStringField(f, &type_name_info, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
			// message
			addMessageField(f, &type_name_info, msg_printer)
		} else {
			log.Fatal("Unsupported field type", f.GetType(), f.GetName())
		}
	} else {
		log.Fatal("Unsupported field label", f.GetLabel(), f.GetName())
	}
}

func holderName(f *descriptor.FieldDescriptorProto) string {
	return f.GetName() + "__"
}

func addPrimitiveField(f *descriptor.FieldDescriptorProto, type_name_info *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"raw_type":    type_name_info.cc_raw_type,
		"ret_type":    type_name_info.cc_ret_type,
		"arg_type":    type_name_info.cc_arg_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}
	msg_printer.PushPrivate(
		print("pri_pri",
			"    {{.raw_type}} {{.holder_name}};\n",
			args))

	msg_printer.PushPublic(
		print("pub_pri", `
	inline {{.ret_type}} {{.f_name}}() const {
	    return {{.holder_name}};
	}

	inline void set_{{.f_name}}( {{.arg_type}} value) {
	    {{.holder_name}} = value;
	}

	inline void clear_{{.f_name}}() {
	    {{.holder_name}} = {{.raw_type}}();
	}
`,
			args))
}

func addStringField(f *descriptor.FieldDescriptorProto, type_name_info *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"raw_type":    type_name_info.cc_raw_type,
		"ret_type":    type_name_info.cc_ret_type,
		"arg_type":    type_name_info.cc_arg_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}
	msg_printer.PushPrivate(
		print("pri_str", "    {{.raw_type}} {{.holder_name}};\n", args))

	msg_printer.PushPublic(
		print("pub_str", `
	inline {{.ret_type}} {{.f_name}}() const {
	    return {{.holder_name}};
	}

	// TODO: Support string_view
	inline void set_{{.f_name}}({{.arg_type}} value) {
	    {{.holder_name}} = value;
	}

	inline void clear_{{.f_name}}() {
	    {{.holder_name}} = {{.raw_type}}();
	}
`,
			args))
}

func addRepeatedPrimitiveField(f *descriptor.FieldDescriptorProto, type_name_info *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"cpp_type":    type_name_info.cc_raw_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}
	msg_printer.PushPrivate(
		print("pri_rep_pri", "    {{.cpp_type}} {{.holder_name}};\n", args))

	msg_printer.PushPublic(
		print("pub_rep_pri", `
	inline const {{.cpp_type}}& {{.f_name}}() const {
	    return {{.holder_name}};
	}

	inline {{.cpp_type}}* mutable_{{.f_name}}() {
		return &{{.holder_name}};
	}

	inline void clear_{{.f_name}}() {
	    {{.holder_name}}.clear();
	}

`,
			args))
}

func addMessageField(f *descriptor.FieldDescriptorProto, type_name *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"raw_type":    type_name.cc_raw_type,
		"ret_type":    type_name.cc_ret_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}
	msg_printer.PushPrivate(
		print("pri_rep_pri",
			"    std::unique_ptr<{{.raw_type}}> {{.holder_name}};\n",
			args))

	msg_printer.PushPublic(
		print("pub_rep_pri",
			`
	{{.ret_type}} {{.f_name}}() const {
	    return *{{.holder_name}};
	}

	{{.raw_type}}* mutable_{{.f_name}}() {
	    if (!{{.holder_name}}) {
			{{.holder_name}} = std::make_unique<{{.raw_type}}>();
        }
	    return {{.holder_name}}.get();
	}

	bool has_{{.f_name}}() const {
	    return (bool){{.holder_name}};
	}

	void clear_{{.f_name}}() {
	    {{.holder_name}}.reset();
	}

`,
			args))
}
