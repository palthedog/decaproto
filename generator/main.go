package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"strings"

	"golang.org/x/exp/maps"

	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
	plugin "github.com/golang/protobuf/protoc-gen-go/plugin"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"
)

func processEnum(ctx *Context, m *descriptor.EnumDescriptorProto) {
	ctx.pushPackage(m.GetName())

	var out string = ""
	full_name := strings.Join(ctx.cpp_nested_pkg, "_")
	out += "enum " + full_name + " : int {\n"
	for _, field := range m.GetValue() {
		out += fmt.Sprintf("    %s = %d,\n", field.GetName(), field.GetNumber())
	}
	out += "};\n\n"
	ctx.printer.enum_definitions += out

	ctx.popPackage()
}

func printReflection(m *descriptor.DescriptorProto, fp *FilePrinter, mp *MessagePrinter) {
	// Declaration
	mp.publics += "    const decaproto::Reflection* GetReflection() const override;\n"

	// Definition
	msg_full_name := mp.full_name
	var singleton_name = "k" + mp.full_name + "__Reflection"
	var src string = ""
	src += "\n"
	src += "// A singleton Reflection object for " + mp.full_name + "\n"
	src += "decaproto::Reflection* " + singleton_name + " = nullptr;\n"
	src += "\n"
	src += "const decaproto::Reflection* " + mp.full_name + "::GetReflection() const {\n"
	src += "    if (" + singleton_name + " != nullptr) {\n"
	src += "        return " + singleton_name + ";\n"
	src += "    }\n"
	src += "    " + singleton_name + " = new decaproto::Reflection();\n"

	for _, f := range m.GetField() {
		src += "    "
		type_name := getTypeNameInfo(f)
		tag_str := fmt.Sprintf("%d", f.GetNumber())
		if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
			if f.GetLabel() != descriptor.FieldDescriptorProto_LABEL_REPEATED {
				src += print("reg_enum_field", `
    // EnumValue setter for {{.field_name}}
     {{.singleton_name}}->RegisterSetEnumValue(
        {{.tag}},
		decaproto::CastForSetEnumValue(&{{.msg_full_name}}::set_{{.field_name}}));
     // EnumValue getter for {{.field_name}}
     {{.singleton_name}}->RegisterGetEnumValue(
        {{.tag}},
		decaproto::CastForGetEnumValue(&{{.msg_full_name}}::{{.field_name}}));
`,
					map[string]string{
						"singleton_name": singleton_name,
						"tag":            tag_str,
						"cc_arg_type":    type_name.cc_arg_type,
						"cc_camel_name":  type_name.cc_camel_name,
						"msg_full_name":  msg_full_name,
						"field_name":     f.GetName(),
					})
			} else {

			}
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE ||
			f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
			// Message and repeated message should be accessed via
			// getter and mutable getter
			src += print("reg_msg_field", `
    // Mutable getter for {{.field_name}}
    {{.singleton_name}}->RegisterMutable{{.cc_camel_name}}(
        {{.tag}},
		decaproto::MsgCast(&{{.msg_full_name}}::mutable_{{.field_name}}));
    // Getter for {{.field_name}}
    {{.singleton_name}}->RegisterGet{{.cc_camel_name}}(
        {{.tag}},
		decaproto::MsgCast(&{{.msg_full_name}}::{{.field_name}}));
`,
				map[string]string{
					"singleton_name": singleton_name,
					"tag":            tag_str,
					"cc_arg_type":    type_name.cc_arg_type,
					"cc_camel_name":  type_name.cc_camel_name,
					"msg_full_name":  msg_full_name,
					"field_name":     f.GetName(),
				})
		} else if isPrimitiveType(f) {
			src += print("reg_field", `
    // Setter
    {{.singleton_name}}->RegisterSet{{.CcType}}(
        {{.tag}},
		decaproto::MsgCast(&{{.msg_full_name}}::set_{{.field_name}}));
    // Getter
    {{.singleton_name}}->RegisterGet{{.CcType}}(
        {{.tag}},
		decaproto::MsgCast(&{{.msg_full_name}}::{{.field_name}}));
    `,
				map[string]string{
					"singleton_name": singleton_name,
					"CcType":         type_name.cc_camel_name,
					"tag":            tag_str,
					"cc_arg_type":    type_name.cc_arg_type,
					"msg_full_name":  msg_full_name,
					"field_name":     f.GetName(),
				})
		}
	}
	src += "    return " + singleton_name + ";\n"
	src += "}\n"

	fp.source_content += src
}

func printDescriptor(m *descriptor.DescriptorProto, fp *FilePrinter, mp *MessagePrinter) {
	// Declaration
	mp.publics += "    const decaproto::Descriptor* GetDescriptor() const override;\n"

	// Definition
	var desc_name = "k" + mp.full_name + "__Descriptor"
	var src string = ""
	src += "\n"
	src += "// A singleton Descriptor for " + mp.full_name + "\n"
	src += "decaproto::Descriptor* " + desc_name + " = nullptr;\n"
	src += "\n"
	src += "const decaproto::Descriptor* " + mp.full_name + "::GetDescriptor() const {\n"
	src += "    if (" + desc_name + " != nullptr) {\n"
	src += "        return " + desc_name + ";\n"
	src += "    }\n"
	src += "    " + desc_name + " = new decaproto::Descriptor();\n"
	for _, f := range m.GetField() {
		tag := f.GetNumber()
		field_type := getTypeNameInfo(f).deca_enum_name
		src += "    "
		src += fmt.Sprintf("%s->RegisterField(decaproto::FieldDescriptor(%d, %s));\n",
			desc_name,
			tag,
			field_type)
	}
	src += "    return " + desc_name + ";\n"
	src += "}\n"

	fp.source_content += src
}

func processMessage(ctx *Context, m *descriptor.DescriptorProto) {
	ctx.pushPackage(m.GetName())

	msg_printer := NewMessagePrinter()
	msg_printer.short_name = m.GetName()
	full_name := strings.Join(ctx.cpp_nested_pkg, "_")
	msg_printer.full_name = full_name

	// nested messages
	for _, nested := range m.GetNestedType() {
		processMessage(ctx, nested)
	}
	// nested enums
	for _, nested := range m.GetEnumType() {
		processEnum(ctx, nested)
	}

	// Define alias for nested messages like:
	// `typedef MyPackage_Nested_MyMessage MyMessage;`
	// so that we can refer to them as `Nested::MyMessage` instead of `MyPackage_Nested_MyMessage`
	for _, nested := range m.GetNestedType() {
		nest_full_name := full_name + "_" + nested.GetName()
		nest_short_name := nested.GetName()
		msg_printer.publics += "    typedef " + nest_full_name + " " + nest_short_name + ";\n"
	}
	// Aliases for nested enums
	for _, nested := range m.GetEnumType() {
		nest_full_name := full_name + "_" + nested.GetName()
		nest_short_name := nested.GetName()
		msg_printer.publics += "    typedef " + nest_full_name + " " + nest_short_name + ";\n"
	}

	ctx.printer.prototype_declarations += "class " + full_name + ";\n"

	for _, field := range m.GetField() {
		processField(ctx, msg_printer, field)
	}
	printDescriptor(m, ctx.printer, msg_printer)
	printReflection(m, ctx.printer, msg_printer)

	ctx.printer.definitions += msg_printer.printClassDefinition()

	ctx.popPackage()
}

func (mp *MessagePrinter) printClassDefinition() string {
	var out string = ""
	out += "class " + mp.full_name + " : public decaproto::Message {\n"
	out += "public:\n"
	out += "    " + mp.full_name + "() {}\n"
	out += "    virtual ~" + mp.full_name + "() {}\n"
	out += "\n"
	out += mp.publics
	out += "\n"
	out += "private:\n"
	out += mp.privates
	out += "\n"
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

type DescriptorPrinter struct {
	tags       []int32
	type_names []string
}

func NewDescriptorPrinter() *DescriptorPrinter {
	return &DescriptorPrinter{
		tags:       []int32{},
		type_names: []string{},
	}
}

type MessagePrinter struct {
	short_name string // MyMessage
	full_name  string // ::MyPackage::Nested::MyMessage

	privates string
	publics  string

	// TODO: We need one descriptor pinters per message
	descriptor_printer *DescriptorPrinter
}

func NewMessagePrinter() *MessagePrinter {
	return &MessagePrinter{
		short_name:         "",
		full_name:          "",
		privates:           "",
		publics:            "",
		descriptor_printer: NewDescriptorPrinter(),
	}
}

func (mp *MessagePrinter) PushPrivate(str string) {
	mp.privates += str
}

func (mp *MessagePrinter) PushPublic(str string) {
	mp.publics += str
}

type FilePrinter struct {
	includes map[string]struct{}

	prototype_declarations string

	enum_definitions string

	definitions string

	source_content string
}

func NewFilePrinter() *FilePrinter {
	return &FilePrinter{
		includes:               map[string]struct{}{},
		prototype_declarations: "",
		enum_definitions:       "",
		definitions:            "",
		source_content:         "",
	}
}

func (fp *FilePrinter) addInclude(inc string) {
	fp.includes[inc] = struct{}{}
}

func (fp *FilePrinter) printIncludes() string {
	keys := maps.Keys(fp.includes)
	return strings.Join(keys, "\n") + "\n"
}

func (fp *FilePrinter) printHeaderFile() string {
	var content string = "#pragma once\n\n"

	content += fp.printIncludes()
	content += "\n"

	content += fp.prototype_declarations
	content += "\n"

	content += fp.enum_definitions
	content += "\n"

	content += fp.definitions
	content += "\n"

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

		if f.GetSyntax() != "proto3" {
			syntax := f.GetSyntax()
			if syntax == "" {
				syntax = "proto2"
			}

			fmt.Fprintf(os.Stderr,
				"WARNING: Decaproto supports proto3 only, but %s is %s\n",
				f.GetName(),
				syntax)
			fmt.Fprintf(os.Stderr, "    Basic functionality should work but "+
				"some behaviours about field presense (e.g. has_*, default value)"+
				"would be different from the original proto2 compiler.\n")
		}

		out_file_name := outputName(f)
		header_file_name := out_file_name + ".h"

		ctx := NewContext()

		ctx.printer.addInclude("#include <memory>")
		ctx.printer.addInclude("#include <stdint.h>")
		ctx.printer.addInclude("#include <string>")
		ctx.printer.addInclude("#include <vector>")
		ctx.printer.addInclude("#include \"decaproto/message.h\"")
		ctx.printer.addInclude("#include \"decaproto/descriptor.h\"")
		ctx.printer.addInclude("#include \"decaproto/reflection.h\"")

		ctx.printer.source_content += "#include \"" + header_file_name + "\"\n"
		ctx.printer.source_content += "\n"
		ctx.printer.source_content += "#include <cassert>\n"
		ctx.printer.source_content += "#include \"decaproto/reflection_util.h\"\n"
		ctx.printer.source_content += "\n"

		for _, enm := range f.EnumType {
			processEnum(ctx, enm)
		}

		for _, message := range f.MessageType {
			processMessage(ctx, message)
		}

		content := ctx.printer.printHeaderFile()

		// Dump all descriptors for reference
		content += "/*\n"
		content += prototext.MarshalOptions{Multiline: true, Indent: "    "}.Format(f)
		content += "*/\n"

		resp.File = append(resp.File, &plugin.CodeGeneratorResponse_File{
			Name:    proto.String(header_file_name),
			Content: proto.String(content),
		})

		resp.File = append(resp.File, &plugin.CodeGeneratorResponse_File{
			Name:    proto.String(out_file_name + ".cc"),
			Content: proto.String(ctx.printer.source_content),
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
