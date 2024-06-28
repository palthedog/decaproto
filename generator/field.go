package main

import (
	"bytes"
	"log"
	"strings"
	"text/template"

	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
)

var primitive_name_map = map[descriptor.FieldDescriptorProto_Type]string{
	descriptor.FieldDescriptorProto_TYPE_INT64:  "int64_t",
	descriptor.FieldDescriptorProto_TYPE_INT32:  "int32_t",
	descriptor.FieldDescriptorProto_TYPE_DOUBLE: "double",
	descriptor.FieldDescriptorProto_TYPE_FLOAT:  "float",
	descriptor.FieldDescriptorProto_TYPE_BOOL:   "bool",
}

func protoMessageTypeToCppType(f *descriptor.FieldDescriptorProto) string {
	cpp_name, ok := primitive_name_map[f.GetType()]
	if ok {
		// primitive types
		return cpp_name
	} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
		return "std::string"
	} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
		// .OtherMessage
		// .OuterMessage.OtherMessage
		m := f.GetTypeName()
		return strings.Join(strings.Split(m, "."), "::")
	} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
		// enum
		m := f.GetTypeName()
		return strings.Join(strings.Split(m, "."), "::")
	} else {
		log.Fatal("Unsupported field type", f.GetType(), f.GetName())
		return ""
	}
}

func isPrimitiveType(f *descriptor.FieldDescriptorProto) bool {
	_, ok := primitive_name_map[f.GetType()]
	return ok
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
