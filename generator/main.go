package main

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
	"text/template"

	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
	plugin "github.com/golang/protobuf/protoc-gen-go/plugin"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"
)

func parseReq(r io.Reader) (*plugin.CodeGeneratorRequest, error) {
	buf, err := io.ReadAll(r)
	if err != nil {
		return nil, err
	}

	var req plugin.CodeGeneratorRequest
	if err = proto.Unmarshal(buf, &req); err != nil {
		return nil, err
	}
	return &req, nil
}

func addPrimitiveField(f *descriptor.FieldDescriptorProto, cpp_type string, pub_out *string, pri_out *string) {
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
	*pri_out = buf.String()

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
		"f_name":   f.GetName(),
	})
	*pub_out = buf.String()
}

func addStringField(f *descriptor.FieldDescriptorProto, cpp_type string, pub_out *string, pri_out *string) {
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
	*pri_out = buf.String()

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
		"f_name":   f.GetName(),
	})
	*pub_out = buf.String()
}

func addRepeatedPrimitiveField(f *descriptor.FieldDescriptorProto, cpp_type string, pub_out *string, pri_out *string) {
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
	*pri_out = buf.String()

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
		"f_name":   f.GetName(),
	})
	*pub_out = buf.String()
}

func addMessageField(f *descriptor.FieldDescriptorProto, cpp_type string, pub_out *string, pri_out *string) {
	const pri_template = `
	std::unique_ptr<{{.cpp_type}}> {{.pri_name}};

 `

	const pub_template = `
	const {{.cpp_type}}& {{.f_name}}() const {
	    return *{{.pri_name}};
	}

	void set_{{.f_name}}(const {{.cpp_type}}& value) {
	    *{{.pri_name}} = value;
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
	*pri_out = buf.String()

	buf.Reset()
	pub_tpl.Execute(&buf, map[string]string{
		"cpp_type": cpp_type,
		"pri_name": pri_name,
		"f_name":   f.GetName(),
	})
	*pub_out = buf.String()
}

var primitive_name_map = map[descriptor.FieldDescriptorProto_Type]string{
	descriptor.FieldDescriptorProto_TYPE_INT64:  "int64_t",
	descriptor.FieldDescriptorProto_TYPE_INT32:  "int32_t",
	descriptor.FieldDescriptorProto_TYPE_DOUBLE: "double",
	descriptor.FieldDescriptorProto_TYPE_FLOAT:  "float",
	descriptor.FieldDescriptorProto_TYPE_BOOL:   "bool",
}

func isPrimitiveType(f *descriptor.FieldDescriptorProto) bool {
	_, ok := primitive_name_map[f.GetType()]
	return ok
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

func processField(f *descriptor.FieldDescriptorProto) (string, string) {
	var pub = ""
	var pri = ""

	cpp_name := protoMessageTypeToCppType(f)
	if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
		// repeated
		if isPrimitiveType(f) {
			// primitive types
			addRepeatedPrimitiveField(f, cpp_name, &pub, &pri)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// string
			addRepeatedPrimitiveField(f, cpp_name, &pub, &pri)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// enum
			addRepeatedPrimitiveField(f, cpp_name, &pub, &pri)
		} else {
			log.Fatal("Unsupported repeated field type", f.GetType(), f.GetName())
		}
	} else if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_OPTIONAL {
		// optional
		if isPrimitiveType(f) {
			// primitive types
			addPrimitiveField(f, cpp_name, &pub, &pri)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// string
			addStringField(f, cpp_name, &pub, &pri)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			// enum
			addPrimitiveField(f, cpp_name, &pub, &pri)
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE {
			// message
			addMessageField(f, cpp_name, &pub, &pri)
		} else {
			log.Fatal("Unsupported field type", f.GetType(), f.GetName())
		}
	} else {
		log.Fatal("Unsupported field label", f.GetLabel(), f.GetName())
	}
	return pub, pri
}

func processEnum(cpp_pkg []string, m *descriptor.EnumDescriptorProto) string {
	var out string = ""

	cpp_nested_pkg := append(cpp_pkg, m.GetName())
	enum_name := strings.Join(cpp_nested_pkg, "_")
	out += "enum " + enum_name + " {\n"
	for _, field := range m.GetValue() {
		out += fmt.Sprintf("    %s = %d,\n", field.GetName(), field.GetNumber())
	}
	out += "};\n\n"
	return out
}

func processMessage(cpp_pkg []string, m *descriptor.DescriptorProto) string {
	var out string = ""

	cpp_nested_pkg := append(cpp_pkg, m.GetName())
	// nested messages
	for _, nested := range m.GetNestedType() {
		out += processMessage(cpp_nested_pkg, nested)
	}
	// nested enums
	for _, nested := range m.GetEnumType() {
		out += processEnum(cpp_nested_pkg, nested)
	}

	var pub_fields = ""
	var pri_fields = ""
	for _, field := range m.GetField() {
		pub, pri := processField(field)
		pub_fields += pub
		pri_fields += pri
	}

	class_name := strings.Join(cpp_nested_pkg, "_")
	out += "class " + class_name + " {\n"
	out += "public:\n"
	// typedef nested messages
	for _, nested := range m.GetNestedType() {
		out += "    typedef " + class_name + "_" + nested.GetName() + " " + nested.GetName() + ";\n"
	}
	// typedef nested enums
	for _, nested := range m.GetEnumType() {
		out += "    typedef " + class_name + "_" + nested.GetName() + " " + nested.GetName() + ";\n"
	}
	out += "private:\n"
	out += pri_fields
	out += "\n"
	out += "public:\n"
	out += "    " + class_name + "() {};\n"
	out += "    ~" + class_name + "() {};\n"
	out += "\n"
	out += pub_fields
	out += "\n};\n\n"
	return out
}

func outputName(f *descriptor.FileDescriptorProto) string {
	strs := strings.Split(f.GetName(), ".")
	if strs[len(strs)-1] == "proto" {
		strs = strs[:len(strs)-1]
		strs = append(strs, "pb")
	}
	return strings.Join(strs, ".")
}

func processReq(req *plugin.CodeGeneratorRequest) *plugin.CodeGeneratorResponse {
	// Build a map of file names to file descriptors.
	files := make(map[string]*descriptor.FileDescriptorProto)
	for _, f := range req.ProtoFile {
		files[f.GetName()] = f
	}

	var resp plugin.CodeGeneratorResponse
	for _, fname := range req.FileToGenerate {
		f := files[fname]
		out_file_name := outputName(f)

		var content = ""
		content += "#include <memory>\n"
		content += "#include <stdint.h>\n"
		content += "#include <string>\n"
		content += "#include <vector>\n"
		content += "\n"

		for _, enm := range f.EnumType {
			content += processEnum([]string{}, enm)
		}

		for _, message := range f.MessageType {
			content += processMessage([]string{}, message)
		}

		content += "\n"
		content += "\n"

		content += "/*\n"
		content += prototext.MarshalOptions{Multiline: true, Indent: "    "}.Format(f)
		content += "*/\n"

		resp.File = append(resp.File, &plugin.CodeGeneratorResponse_File{
			Name:    proto.String(out_file_name + ".h"),
			Content: proto.String(content),
		})

		resp.File = append(resp.File, &plugin.CodeGeneratorResponse_File{
			Name:    proto.String(out_file_name + ".cc"),
			Content: proto.String("#include \"" + out_file_name + ".h\"\n"),
		})
	}
	return &resp
}

func emitResp(resp *plugin.CodeGeneratorResponse) error {
	buf, err := proto.Marshal(resp)
	if err != nil {
		return err
	}
	_, err = os.Stdout.Write(buf)
	return err
}

func run() error {
	req, err := parseReq(os.Stdin)
	if err != nil {
		return err
	}

	resp := processReq(req)

	return emitResp(resp)
}

func main() {
	if err := run(); err != nil {
		log.Fatalln(err)
	}
}
