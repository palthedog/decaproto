package main

import (
	"bytes"
	"log"
	"strings"
	"text/template"

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

var type_name_infos_map = map[descriptor.FieldDescriptorProto_Type]TypeNameInfo{
	descriptor.FieldDescriptorProto_TYPE_UINT64:  NewPrimitiveTypeNameInfo("uint64", "kUInt64", "uint64_t", "UInt64"),
	descriptor.FieldDescriptorProto_TYPE_INT64:   NewPrimitiveTypeNameInfo("int64", "kInt64", "int64_t", "Int64"),
	descriptor.FieldDescriptorProto_TYPE_FIXED64: NewPrimitiveTypeNameInfo("fixed64", "kFixed64", "uint64_t", "UInt64"),
	descriptor.FieldDescriptorProto_TYPE_UINT32:  NewPrimitiveTypeNameInfo("uint32", "kUInt32", "uint32_t", "UInt32"),
	descriptor.FieldDescriptorProto_TYPE_INT32:   NewPrimitiveTypeNameInfo("int32", "kInt32", "int32_t", "Int32"),
	descriptor.FieldDescriptorProto_TYPE_FIXED32: NewPrimitiveTypeNameInfo("fixed32", "kFixed32", "uint32_t", "UInt32"),
	descriptor.FieldDescriptorProto_TYPE_DOUBLE:  NewPrimitiveTypeNameInfo("double", "kDouble", "double", "Double"),
	descriptor.FieldDescriptorProto_TYPE_FLOAT:   NewPrimitiveTypeNameInfo("float", "kFloat", "float", "Float"),
	descriptor.FieldDescriptorProto_TYPE_BOOL:    NewPrimitiveTypeNameInfo("bool", "kBool", "bool", "Bool"),
	descriptor.FieldDescriptorProto_TYPE_STRING:  NewObjectTypeNameInfo("string", "kString", "std::string", "String"),

	descriptor.FieldDescriptorProto_TYPE_MESSAGE: NewGeneratedTypeNameInfo("kMessage", "Message"),
	// We use "EnumValue" because cc_camel_name is used only for the getter/setter in Reflection
	// and our Reflection interface provides only enum accessors via EnumValue(i.e. int)
	descriptor.FieldDescriptorProto_TYPE_ENUM: NewGeneratedTypeNameInfo("kEnum", "EnumValue"),
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
		descriptor.FieldDescriptorProto_TYPE_BOOL,
		descriptor.FieldDescriptorProto_TYPE_STRING:
		return true
	}
	return false
}

func processField(ctx *Context, msg_printer *MessagePrinter, f *descriptor.FieldDescriptorProto) {
	type_name_info := getTypeNameInfo(f)
	// TODO: Replace it with type_name_info so that we can share more logic across
	// different types of fields
	cpp_name := type_name_info.cc_raw_type
	if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
		// repeated
		if isPrimitiveType(f) {
			// primitive types
			addRepeatedPrimitiveField(f, cpp_name, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// string
			addRepeatedPrimitiveField(f, cpp_name, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// enum
			addRepeatedPrimitiveField(f, cpp_name, msg_printer)
		} else {
			log.Fatal("Unsupported repeated field type", f.GetType(), f.GetName())
		}
	} else if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_OPTIONAL {
		// optional
		if isPrimitiveType(f) {
			// primitive types
			addPrimitiveField(f, cpp_name, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// string
			addStringField(f, cpp_name, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// enum
			addPrimitiveField(f, cpp_name, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
			// message
			addMessageField(f, type_name_info, msg_printer)
		} else {
			log.Fatal("Unsupported field type", f.GetType(), f.GetName())
		}
	} else {
		log.Fatal("Unsupported field label", f.GetLabel(), f.GetName())
	}
}

func addPrimitiveField(f *descriptor.FieldDescriptorProto, cpp_type string, msg_printer *MessagePrinter) {
	const pri_template = `
	{{.cpp_type}} {{.pri_name}};
	bool has_{{.pri_name}};

`

	const pub_template = `
	inline {{.cpp_type}} {{.f_name}}() const {
	    return {{.pri_name}};
	}

	inline void set_{{.f_name}}( {{.cpp_type}} value) {
	    {{.pri_name}} = value;
	    has_{{.pri_name}} = true;
	}

	inline bool has_{{.f_name}}() const {
	    return has_{{.pri_name}};
	}

	inline void clear_{{.f_name}}() {
	    // TODO: Support default value
	    {{.pri_name}} = {{.cpp_type}}();
	    has_{{.pri_name}} = false;
	}

`

	pri_tpl, err := template.New("pri").Parse(pri_template)
	if err != nil {
		log.Fatal(err)
	}
	pub_tpl, err := template.New("pub").Parse(pub_template)
	if err != nil {
		log.Fatal(err)
	}

	var pri_name = f.GetName() + "__"
	var buf bytes.Buffer
	pri_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
	})
	msg_printer.PushPrivate(buf.String())

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
		"f_name":   f.GetName(),
	})
	msg_printer.PushPublic(buf.String())
}

func addStringField(f *descriptor.FieldDescriptorProto, cpp_type string, msg_printer *MessagePrinter) {
	const pri_template = `
	{{.cpp_type}} {{.pri_name}};
	bool has_{{.pri_name}};

`

	const pub_template = `
	inline {{.cpp_type}} {{.f_name}}() const {
	    return {{.pri_name}};
	}

	// TODO: Support string_view
	inline void set_{{.f_name}}(const {{.cpp_type}}& value) {
	    {{.pri_name}} = value;
	    has_{{.pri_name}} = true;
	}

	inline bool has_{{.f_name}}() const {
	    return has_{{.pri_name}};
	}

	inline void clear_{{.f_name}}() {
	    // TODO: Support default value
	    {{.pri_name}} = {{.cpp_type}}();
	    has_{{.pri_name}} = false;
	}

`

	pri_tpl, err := template.New("pri").Parse(pri_template)
	if err != nil {
		log.Fatal(err)
	}
	pub_tpl, err := template.New("pub").Parse(pub_template)
	if err != nil {
		log.Fatal(err)
	}

	var pri_name = f.GetName() + "__"
	var buf bytes.Buffer
	pri_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
	})
	msg_printer.PushPrivate(buf.String())

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
		"f_name":   f.GetName(),
	})
	msg_printer.PushPublic(buf.String())
}

func addRepeatedPrimitiveField(f *descriptor.FieldDescriptorProto, cpp_type string, msg_printer *MessagePrinter) {
	const pri_template = `
	{{.cpp_type}} {{.pri_name}};

`
	const pub_template = `
	inline const {{.cpp_type}}& {{.f_name}}() const {
	    return {{.pri_name}};
	}

	inline {{.cpp_type}}* mutable_{{.f_name}}() {
		return &{{.pri_name}};
	}

	inline void clear_{{.f_name}}() {
	    {{.pri_name}}.clear();
	}

`

	pri_tpl, err := template.New("pri").Parse(pri_template)
	if err != nil {
		log.Fatal(err)
	}
	pub_tpl, err := template.New("pub").Parse(pub_template)
	if err != nil {
		log.Fatal(err)
	}

	var pri_name = f.GetName() + "__"
	var buf bytes.Buffer
	pri_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
	})
	msg_printer.PushPrivate(buf.String())

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
		"f_name":   f.GetName(),
	})
	msg_printer.PushPublic(buf.String())
}

func addMessageField(f *descriptor.FieldDescriptorProto, type_name TypeNameInfo, msg_printer *MessagePrinter) {
	const pri_template = `
	std::unique_ptr<{{.cpp_type}}> {{.pri_name}};

 `

	const pub_template = `
	{{.cpp_ret_type}} {{.f_name}}() const {
	    return *{{.pri_name}};
	}

	{{.cpp_type}}* mutable_{{.f_name}}() {
	    if (!{{.pri_name}}) {
			{{.pri_name}} = std::make_unique<{{.cpp_type}}>();
        }
	    return {{.pri_name}}.get();
	}

	bool has_{{.f_name}}() const {
	    return (bool){{.pri_name}};
	}

	void clear_{{.f_name}}() {
	    {{.pri_name}}.reset();
	}

 `

	pri_tpl, err := template.New("pri").Parse(pri_template)
	if err != nil {
		log.Fatal(err)
	}
	pub_tpl, err := template.New("pub").Parse(pub_template)
	if err != nil {
		log.Fatal(err)
	}

	var pri_name = f.GetName() + "__"
	var buf bytes.Buffer
	pri_tpl.Execute(&buf, map[string]string{
		"cpp_type":     type_name.cc_raw_type,
		"cpp_ret_type": type_name.cc_ret_type,
		"pri_name":     pri_name,
	})
	msg_printer.PushPrivate(buf.String())

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type":     type_name.cc_raw_type,
		"cpp_ret_type": type_name.cc_ret_type,
		"pri_name":     pri_name,
		"f_name":       f.GetName(),
	})
	msg_printer.PushPublic(buf.String())
}
