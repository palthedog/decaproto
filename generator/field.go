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

func NewTypeNameInfo(proto_name, deca_enum_name, cc_ret_type, cc_arg_type, cc_camel_name string) TypeNameInfo {
	return TypeNameInfo{
		proto_name:     proto_name,
		deca_enum_name: "decaproto::FieldType::" + deca_enum_name,
		cc_ret_type:    cc_ret_type,
		cc_arg_type:    cc_arg_type,
		cc_camel_name:  cc_camel_name,
	}
}

var type_name_infos_map = map[descriptor.FieldDescriptorProto_Type]TypeNameInfo{
	descriptor.FieldDescriptorProto_TYPE_UINT64:  NewTypeNameInfo("uint64", "kUInt64", "uint64_t", "uint64_t", "UInt64"),
	descriptor.FieldDescriptorProto_TYPE_INT64:   NewTypeNameInfo("int64", "kInt64", "int64_t", "int64_t", "Int64"),
	descriptor.FieldDescriptorProto_TYPE_FIXED64: NewTypeNameInfo("fixed64", "kFixed64", "uint64_t", "uint64_t", "UInt64"),
	descriptor.FieldDescriptorProto_TYPE_UINT32:  NewTypeNameInfo("uint32", "kUInt32", "uint32_t", "uint32_t", "UInt32"),
	descriptor.FieldDescriptorProto_TYPE_INT32:   NewTypeNameInfo("int32", "kInt32", "int32_t", "int32_t", "Int32"),
	descriptor.FieldDescriptorProto_TYPE_FIXED32: NewTypeNameInfo("fixed32", "kFixed32", "uint32_t", "uint32_t", "UInt32"),
	descriptor.FieldDescriptorProto_TYPE_DOUBLE:  NewTypeNameInfo("double", "kDouble", "double", "double", "Double"),
	descriptor.FieldDescriptorProto_TYPE_FLOAT:   NewTypeNameInfo("float", "kFloat", "float", "float", "Float"),
	descriptor.FieldDescriptorProto_TYPE_BOOL:    NewTypeNameInfo("bool", "kBool", "bool", "bool", "Bool"),
	descriptor.FieldDescriptorProto_TYPE_STRING:  NewTypeNameInfo("string", "kString", "std::string", "const std::string&", "String"),

	descriptor.FieldDescriptorProto_TYPE_MESSAGE: NewTypeNameInfo("N_A_FILL_ME", "kMessage", "N_A_FILL_ME", "N_A_FILL_ME", "N_A_FILL_ME_Message"),
	descriptor.FieldDescriptorProto_TYPE_ENUM:    NewTypeNameInfo("N_A_FILL_ME", "kEnum", "N_A_FILL_ME", "N_A_FILL_ME", "N_A_FILL_ME_Enum"),
}

func getTypeNameInfo(f *descriptor.FieldDescriptorProto) TypeNameInfo {
	info, ok := type_name_infos_map[f.GetType()]
	if !ok {
		// TODO: Support message
		// we can create TypeInfoName struct here for messages
		log.Fatal("Unsupported field type", f.GetType(), f.GetName())
	}
	return info
}

// TODO: Get rid of it.Return TypeNameInfo instead (even for Messges)
//  otherwise, we can't generate setters/getters in proper way
func protoMessageTypeToCppType(f *descriptor.FieldDescriptorProto) string {
	if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
		// .OtherMessage
		// .OuterMessage.OtherMessage
		m := f.GetTypeName()
		return strings.Join(strings.Split(m, "."), "::")
	} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
		// enum
		m := f.GetTypeName()
		return strings.Join(strings.Split(m, "."), "::")
	}

	cpp_name, ok := type_name_infos_map[f.GetType()]
	if ok {
		// primitive types
		return cpp_name.cc_ret_type
	} else {
		log.Fatal("Unsupported field type", f.GetType(), f.GetName())
		return ""
	}
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
	cpp_name := protoMessageTypeToCppType(f)
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
			addMessageField(f, cpp_name, msg_printer)
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
	std::vector<{{.cpp_type}}> {{.pri_name}};

`
	const pub_template = `
	inline const std::vector<{{.cpp_type}}>& {{.f_name}}() const {
	    return {{.pri_name}};
	}

	inline std::vector<{{.cpp_type}}>* mutable_{{.f_name}}() {
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

func addMessageField(f *descriptor.FieldDescriptorProto, cpp_type string, msg_printer *MessagePrinter) {
	const pri_template = `
	std::unique_ptr<{{.cpp_type}}> {{.pri_name}};

 `

	const pub_template = `
	const {{.cpp_type}}& {{.f_name}}() const {
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
