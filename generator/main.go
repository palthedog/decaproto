package main

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
	"text/template"

	"golang.org/x/exp/maps"

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

func processEnum(ctx *Context, m *descriptor.EnumDescriptorProto) {
	var out string = ""

	ctx.pushPackage(m.GetName())

	full_name := strings.Join(ctx.cpp_nested_pkg, "_")
	out += "enum " + full_name + " {\n"
	for _, field := range m.GetValue() {
		out += fmt.Sprintf("    %s = %d,\n", field.GetName(), field.GetNumber())
	}
	out += "};\n\n"

	ctx.popPackage()

	ctx.printer.prototype_declarations += "enum " + full_name + ";\n"
	ctx.printer.definitions += out
}

func processMessage(ctx *Context, m *descriptor.DescriptorProto) {
	msg_printer := NewMessagePrinter()

	msg_printer.short_name = m.GetName()
	ctx.pushPackage(m.GetName())

	// nested messages
	for _, nested := range m.GetNestedType() {
		processMessage(ctx, nested)
	}
	// nested enums
	for _, nested := range m.GetEnumType() {
		processEnum(ctx, nested)
	}

	msg_printer.full_name = strings.Join(ctx.cpp_nested_pkg, "_")

	ctx.printer.prototype_declarations += "class " + msg_printer.full_name + ";\n"

	for _, field := range m.GetField() {
		processField(ctx, msg_printer, field)
	}

	var out string = ""
	class_name := strings.Join(ctx.cpp_nested_pkg, "_")

	out += "class " + msg_printer.full_name + " {\n"
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
	out += msg_printer.fields_printer.PrintPrivates()
	out += "\n"
	out += "public:\n"
	out += "    " + class_name + "() {};\n"
	out += "    ~" + class_name + "() {};\n"
	out += "\n"
	out += msg_printer.fields_printer.PrintPublics()
	out += "\n};\n\n"

	ctx.printer.definitions += out

	ctx.popPackage()
}

func outputName(f *descriptor.FileDescriptorProto) string {
	strs := strings.Split(f.GetName(), ".")
	if strs[len(strs)-1] == "proto" {
		strs = strs[:len(strs)-1]
		strs = append(strs, "pb")
	}
	return strings.Join(strs, ".")
}

type Context struct {
	printer        *FilePrinter
	cpp_nested_pkg []string
}

func NewContext() *Context {
	return &Context{
		printer:        NewFilePrinter(),
		cpp_nested_pkg: []string{},
	}
}

func (ctx *Context) pushPackage(pkg string) {
	ctx.cpp_nested_pkg = append(ctx.cpp_nested_pkg, pkg)
}

func (ctx *Context) popPackage() {
	ctx.cpp_nested_pkg = ctx.cpp_nested_pkg[:len(ctx.cpp_nested_pkg)-1]
}

type MessagePrinter struct {
	short_name     string // MyMessage
	full_name      string // ::MyPackage::Nested::MyMessage
	fields_printer *FieldsPrinter
}

func NewMessagePrinter() *MessagePrinter {
	return &MessagePrinter{
		short_name:     "",
		full_name:      "",
		fields_printer: NewFieldsPrinter(),
	}
}

func (mp *MessagePrinter) PushPrivate(str string) {
	mp.fields_printer.PushPrivate(str)
}

func (mp *MessagePrinter) PushPublic(str string) {
	mp.fields_printer.PushPublic(str)
}

type FilePrinter struct {
	includes map[string]struct{}

	prototype_declarations string
	definitions            string
}

func NewFilePrinter() *FilePrinter {
	return &FilePrinter{
		includes:               map[string]struct{}{},
		prototype_declarations: "",
		definitions:            "",
	}
}

func (fp *FilePrinter) addInclude(inc string) {
	fp.includes[inc] = struct{}{}
}

func (fp *FilePrinter) printIncludes() string {
	keys := maps.Keys(fp.includes)
	return strings.Join(keys, "\n")
}

func (fp *FilePrinter) printFile() string {
	var content string = ""

	content += fp.printIncludes()

	content += fp.prototype_declarations
	content += fp.definitions

	return content
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

		ctx := NewContext()

		ctx.printer.addInclude("#include <memory>")
		ctx.printer.addInclude("#include <stdint.h>")
		ctx.printer.addInclude("#include <string>")
		ctx.printer.addInclude("#include <vector>")

		//
		for _, enm := range f.EnumType {
			processEnum(ctx, enm)
		}

		for _, message := range f.MessageType {
			processMessage(ctx, message)
		}

		content := ctx.printer.printFile()

		// Dump all descriptors for reference
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
