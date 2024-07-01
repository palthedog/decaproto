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
	out += "class " + mp.full_name + " final : public decaproto::Message {\n"
	out += "public:\n"

	// Constructor
	out += "    " + mp.full_name + "()\n"
	if len(mp.init_default_values) > 0 {
		out += "\n        : "
		out += strings.Join(mp.init_default_values, "\n        , ")
		out += "\n"
	}
	out += "{}\n"

	// Destructor
	out += "    ~" + mp.full_name + "() {}\n"
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

	init_default_values []string

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

func (mp *MessagePrinter) PushInitializer(str string) {
	mp.init_default_values = append(mp.init_default_values, str)
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
