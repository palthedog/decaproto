package main

import (
	"fmt"
	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
)

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
