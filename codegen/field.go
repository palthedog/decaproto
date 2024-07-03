package main

import (
	"log"
	"strings"

	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
	"golang.org/x/text/cases"
	"golang.org/x/text/language"
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
	cc_type string

	// for camel case C++ type
	// e.g. UInt64, Int32, String
	cc_camel_name string
}

func NewPrimitiveTypeNameInfo(proto_name, cc_type string) TypeNameInfo {
	camel_name := cases.Title(language.English, cases.Compact).String(proto_name)
	return TypeNameInfo{
		proto_name:     proto_name,
		deca_enum_name: "decaproto::FieldType::k" + camel_name,
		cc_type:        cc_type,
		cc_camel_name:  camel_name,
	}
}

func NewObjectTypeNameInfo(proto_name, deca_enum_name, cc_type, cc_camel_name string) TypeNameInfo {
	return TypeNameInfo{
		proto_name:     proto_name,
		deca_enum_name: "decaproto::FieldType::" + deca_enum_name,
		cc_type:        cc_type,
		cc_camel_name:  cc_camel_name,
	}
}

func NewGeneratedTypeNameInfo(deca_enum_name, cc_camel_name string) TypeNameInfo {
	n_a := "NOT_AVAILABLE_YET_FILL_ME"
	return TypeNameInfo{
		proto_name:     n_a,
		deca_enum_name: "decaproto::FieldType::" + deca_enum_name,
		cc_type:        n_a,
		cc_camel_name:  cc_camel_name,
	}
}

func getTypeNameInfoBase(f *descriptor.FieldDescriptorProto) TypeNameInfo {
	switch f.GetType() {
	case descriptor.FieldDescriptorProto_TYPE_UINT64:
		return NewPrimitiveTypeNameInfo("uint64", "uint64_t")
	case descriptor.FieldDescriptorProto_TYPE_INT64:
		return NewPrimitiveTypeNameInfo("int64", "int64_t")
	case descriptor.FieldDescriptorProto_TYPE_FIXED64:
		return NewPrimitiveTypeNameInfo("fixed64", "uint64_t")
	case descriptor.FieldDescriptorProto_TYPE_SFIXED64:
		return NewPrimitiveTypeNameInfo("sfixed64", "int64_t")
	case descriptor.FieldDescriptorProto_TYPE_UINT32:
		return NewPrimitiveTypeNameInfo("uint32", "uint32_t")
	case descriptor.FieldDescriptorProto_TYPE_INT32:
		return NewPrimitiveTypeNameInfo("int32", "int32_t")
	case descriptor.FieldDescriptorProto_TYPE_SINT32:
		return NewPrimitiveTypeNameInfo("sint32", "int32_t")
	case descriptor.FieldDescriptorProto_TYPE_SINT64:
		return NewPrimitiveTypeNameInfo("sint64", "int64_t")
	case descriptor.FieldDescriptorProto_TYPE_FIXED32:
		return NewPrimitiveTypeNameInfo("fixed32", "uint32_t")
	case descriptor.FieldDescriptorProto_TYPE_SFIXED32:
		return NewPrimitiveTypeNameInfo("sfixed32", "int32_t")
	case descriptor.FieldDescriptorProto_TYPE_DOUBLE:
		return NewPrimitiveTypeNameInfo("double", "double")
	case descriptor.FieldDescriptorProto_TYPE_FLOAT:
		return NewPrimitiveTypeNameInfo("float", "float")
	case descriptor.FieldDescriptorProto_TYPE_BOOL:
		return NewPrimitiveTypeNameInfo("bool", "bool")
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
		cc_type := strings.Join(strings.Split(proto_type_name, "."), "::")
		// Access Message objects via const reference
		info.proto_name = proto_type_name
		info.cc_type = cc_type
	} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
		// .MyEnum
		// .OuterMessage.NestedEnum
		proto_type_name := f.GetTypeName()
		// ::OuterMessage::NestedEnum
		cc_type := strings.Join(strings.Split(proto_type_name, "."), "::")
		// Enum is copyable
		info.proto_name = proto_type_name
		info.cc_type = cc_type
	}
	return info
}

func isPrimitiveType(f *descriptor.FieldDescriptorProto) bool {
	switch f.GetType() {
	case descriptor.FieldDescriptorProto_TYPE_UINT64,
		descriptor.FieldDescriptorProto_TYPE_INT64,
		descriptor.FieldDescriptorProto_TYPE_FIXED64,
		descriptor.FieldDescriptorProto_TYPE_SFIXED64,
		descriptor.FieldDescriptorProto_TYPE_SINT64,
		descriptor.FieldDescriptorProto_TYPE_UINT32,
		descriptor.FieldDescriptorProto_TYPE_INT32,
		descriptor.FieldDescriptorProto_TYPE_FIXED32,
		descriptor.FieldDescriptorProto_TYPE_SFIXED32,
		descriptor.FieldDescriptorProto_TYPE_SINT32,
		descriptor.FieldDescriptorProto_TYPE_DOUBLE,
		descriptor.FieldDescriptorProto_TYPE_FLOAT,
		descriptor.FieldDescriptorProto_TYPE_BOOL:
		return true
	}
	return false
}

func processField(msg_printer *MessagePrinter, f *descriptor.FieldDescriptorProto) {
	type_name_info := getTypeNameInfo(f)
	if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
		// repeated
		if isPrimitiveType(f) || f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// primitive types, enum
			addRepeatedPrimitiveField(f, &type_name_info, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING ||
			f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
			// string, message
			addRepeatedObjectField(f, &type_name_info, msg_printer)
		} else {
			log.Fatal("Unsupported repeated field type: ", f.GetType(), f.GetName())
		}
	} else if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_OPTIONAL {
		// optional
		if isPrimitiveType(f) || f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// primitive types,  enum
			addPrimitiveField(f, &type_name_info, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// string
			addStringField(f, &type_name_info, msg_printer)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
			// message
			addMessageField(f, &type_name_info, msg_printer)
		} else {
			log.Fatal("Unsupported field type ", f.GetType(), f.GetName())
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
		"cc_type":     type_name_info.cc_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}
	msg_printer.PushInitializer(
		print("init_default_values", "{{.holder_name}}({{.cc_type}}())", args))

	msg_printer.PushPrivate(
		print("pri_pri",
			"    {{.cc_type}} {{.holder_name}};\n",
			args))

	msg_printer.PushPublic(
		print("pub_pri", `
	inline {{.cc_type}} {{.f_name}}() const {
	    return {{.holder_name}};
	}

	inline void set_{{.f_name}}( {{.cc_type}} value) {
	    {{.holder_name}} = value;
	}

	inline void clear_{{.f_name}}() {
	    {{.holder_name}} = {{.cc_type}}();
	}
`,
			args))
}

func addStringField(f *descriptor.FieldDescriptorProto, type_name_info *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"cc_type":     type_name_info.cc_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}
	msg_printer.PushInitializer(
		print("init_default_values", "{{.holder_name}}({{.cc_type}}())", args))

	msg_printer.PushPrivate(
		print("pri_str", "    {{.cc_type}} {{.holder_name}};\n", args))

	msg_printer.PushPublic(
		print("pub_str", `
	inline const {{.cc_type}}& {{.f_name}}() const {
	    return {{.holder_name}};
	}

	// TODO: Support string_view
	inline void set_{{.f_name}}(const {{.cc_type}}& value) {
	    {{.holder_name}} = value;
	}

	inline std::string* mutable_{{.f_name}}() {
	    return &{{.holder_name}};
	}

	inline void clear_{{.f_name}}() {
	    {{.holder_name}}.clear();
	}
`,
			args))
}

func addRepeatedPrimitiveField(f *descriptor.FieldDescriptorProto, type_name_info *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"cc_type":     type_name_info.cc_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}

	msg_printer.PushInitializer(
		print("init_default_values", "{{.holder_name}}()", args))

	msg_printer.PushPrivate(
		print("pri_rep_pri", "    std::vector<{{.cc_type}}> {{.holder_name}};\n", args))

	msg_printer.PushPublic(
		print("pub_rep_pri", `
	inline const std::vector<{{.cc_type}}>& {{.f_name}}() const {
	    return {{.holder_name}};
	}

	inline {{.cc_type}} get_{{.f_name}}(size_t index) const {
	    return {{.holder_name}}[index];
	}

	inline size_t {{.f_name}}_size() const {
	    return {{.holder_name}}.size();
	}

	inline void set_{{.f_name}}(size_t index, {{.cc_type}} value) {
	    {{.holder_name}}[index] = value;
	}

	inline std::vector<{{.cc_type}}>* mutable_{{.f_name}}() {
		return &{{.holder_name}};
	}

	inline {{.cc_type}}* add_{{.f_name}}() {
	    {{.holder_name}}.push_back({{.cc_type}}());
		return &{{.holder_name}}.back();
	}

	inline void clear_{{.f_name}}() {
	    {{.holder_name}}.clear();
	}

`,
			args))
}

func addRepeatedObjectField(f *descriptor.FieldDescriptorProto, type_name_info *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"cc_type":     type_name_info.cc_type,
		"holder_name": holderName(f),
		"f_name":      f.GetName(),
	}

	msg_printer.PushInitializer(
		print("init_default_values", "{{.holder_name}}()", args))

	msg_printer.PushPrivate(
		print("pri_rep_pri", "    std::vector<{{.cc_type}}> {{.holder_name}};\n", args))

	msg_printer.PushPublic(
		print("pub_rep_pri", `
	inline const std::vector<{{.cc_type}}>& {{.f_name}}() const {
	    return {{.holder_name}};
	}

	inline const {{.cc_type}}& get_{{.f_name}}(size_t index) const {
	    return {{.holder_name}}[index];
	}

	inline size_t {{.f_name}}_size() const {
	    return {{.holder_name}}.size();
	}

	inline void set_{{.f_name}}(size_t index, const {{.cc_type}}& value) {
	    {{.holder_name}}[index] = value;
	}

	inline std::vector<{{.cc_type}}>* mutable_{{.f_name}}() {
		return &{{.holder_name}};
	}

	inline {{.cc_type}}* add_{{.f_name}}() {
	    {{.holder_name}}.push_back({{.cc_type}}());
		return &{{.holder_name}}.back();
	}

	inline void clear_{{.f_name}}() {
	    {{.holder_name}}.clear();
	}

`,
			args))
}

func addMessageField(f *descriptor.FieldDescriptorProto, type_name *TypeNameInfo, msg_printer *MessagePrinter) {
	args := map[string]string{
		"cc_type":         type_name.cc_type,
		"holder_name":     holderName(f),
		"has_holder_name": "has_" + holderName(f),
		"f_name":          f.GetName(),
	}

	msg_printer.PushInitializer(
		print("init_default_values", "{{.holder_name}}()", args))
	msg_printer.PushInitializer(
		print("init_default_has_values", "{{.has_holder_name}}(false)", args))

	msg_printer.PushPrivate(
		print("pri_rep_pri",
			"    mutable decaproto::SubMessagePtr<{{.cc_type}}> {{.holder_name}};\n"+
				"    bool {{.has_holder_name}};\n",
			args))

	msg_printer.PushPublic(
		print("pub_rep_pri",
			`
	// Getter for {{.f_name}}
	const {{.cc_type}}& {{.f_name}}() const {
		if (!{{.holder_name}}) {
			//{{.holder_name}}.reset(new {{.cc_type}}());
			{{.holder_name}}.resetDefault();
        }
	    return *{{.holder_name}};
	}

	// Mutable Getter for {{.f_name}}
	{{.cc_type}}* mutable_{{.f_name}}() {
	    if (!{{.holder_name}}) {
			//{{.holder_name}}.reset(new {{.cc_type}}());
			{{.holder_name}}.resetDefault();
        }
        {{.has_holder_name}} = true;
	    return {{.holder_name}}.get();
	}

	// Hazzer for {{.f_name}}
	bool has_{{.f_name}}() const {
	    return {{.has_holder_name}};
	}

	// Clearer for {{.f_name}}
	void clear_{{.f_name}}() {
	    {{.holder_name}}.reset();
		{{.has_holder_name}} = false;
	}

`,
			args))
}
