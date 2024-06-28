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

	ctx.printer.enum_definitions += out
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

	full_name := strings.Join(ctx.cpp_nested_pkg, "_")
	msg_printer.full_name = full_name

	ctx.printer.prototype_declarations += "class " + msg_printer.full_name + ";\n"

	for _, field := range m.GetField() {
		processField(ctx, msg_printer, field)
	}

	var out string = ""

	out += "class " + full_name + " {\n"
	out += "public:\n"
	// typedef nested messages
	for _, nested := range m.GetNestedType() {
		nest_full_name := full_name + "_" + nested.GetName()
		nest_short_name := nested.GetName()
		out += "    typedef " + nest_full_name + " " + nest_short_name + ";\n"
	}
	// typedef nested enums
	for _, nested := range m.GetEnumType() {
		nest_full_name := full_name + "_" + nested.GetName()
		nest_short_name := nested.GetName()
		out += "    typedef " + nest_full_name + " " + nest_short_name + ";\n"
	}
	out += "private:\n"
	out += msg_printer.privates
	out += "\n"
	out += "public:\n"
	out += "    " + full_name + "() {};\n"
	out += "    ~" + full_name + "() {};\n"
	out += "\n"
	out += msg_printer.publics
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
	short_name string // MyMessage
	full_name  string // ::MyPackage::Nested::MyMessage

	privates string
	publics  string
}

func NewMessagePrinter() *MessagePrinter {
	return &MessagePrinter{
		short_name: "",
		full_name:  "",
		privates:   "",
		publics:    "",
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
}

func NewFilePrinter() *FilePrinter {
	return &FilePrinter{
		includes:               map[string]struct{}{},
		prototype_declarations: "",
		enum_definitions:       "",
		definitions:            "",
	}
}

func (fp *FilePrinter) addInclude(inc string) {
	fp.includes[inc] = struct{}{}
}

func (fp *FilePrinter) printIncludes() string {
	keys := maps.Keys(fp.includes)
	return strings.Join(keys, "\n") + "\n"
}

func (fp *FilePrinter) printFile() string {
	var content string = ""

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
		out_file_name := outputName(f)

		ctx := NewContext()

		ctx.printer.addInclude("#include <memory>")
		ctx.printer.addInclude("#include <stdint.h>")
		ctx.printer.addInclude("#include <string>")
		ctx.printer.addInclude("#include <vector>")

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
