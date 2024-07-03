package main

import (
	"fmt"
	"os"

	descriptor "github.com/golang/protobuf/protoc-gen-go/descriptor"
)

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

		if f.GetLabel() == descriptor.FieldDescriptorProto_LABEL_REPEATED {
			if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
				src += print("reg_repeated_field", `
			// Mutable getter for {{.field_name}}
			{{.singleton_name}}->RegisterGetRepeated{{.cc_camel_name}}(
				{{.tag}},
				decaproto::MsgCast(&{{.msg_full_name}}::get_{{.field_name}}));
			{{.singleton_name}}->RegisterAddRepeated{{.cc_camel_name}}(
				{{.tag}},
				decaproto::CastForAddRepeatedEnumValue(&{{.msg_full_name}}::add_{{.field_name}}));
			{{.singleton_name}}->RegisterFieldSize(
				{{.tag}},
				decaproto::MsgCast(&{{.msg_full_name}}::{{.field_name}}_size));
		`,
					map[string]string{
						"singleton_name": singleton_name,
						"cc_camel_name":  type_name.cc_camel_name,
						"tag":            tag_str,
						"msg_full_name":  msg_full_name,
						"field_name":     f.GetName(),
					})
			} else {
				src += print("reg_repeated_field", `
			// Mutable getter for {{.field_name}}
			{{.singleton_name}}->RegisterGetRepeated{{.cc_camel_name}}(
				{{.tag}},
				decaproto::MsgCast(&{{.msg_full_name}}::get_{{.field_name}}));
			{{.singleton_name}}->RegisterAddRepeated{{.cc_camel_name}}(
				{{.tag}},
				decaproto::MsgCast(&{{.msg_full_name}}::add_{{.field_name}}));
			{{.singleton_name}}->RegisterFieldSize(
				{{.tag}},
				decaproto::MsgCast(&{{.msg_full_name}}::{{.field_name}}_size));
		`,
					map[string]string{
						"singleton_name": singleton_name,
						"cc_camel_name":  type_name.cc_camel_name,
						"tag":            tag_str,
						"msg_full_name":  msg_full_name,
						"field_name":     f.GetName(),
					})
			}
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_ENUM {
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
					"msg_full_name":  msg_full_name,
					"field_name":     f.GetName(),
				})
		} else if f.GetType() == descriptor.FieldDescriptorProto_TYPE_MESSAGE || f.GetType() == descriptor.FieldDescriptorProto_TYPE_STRING {
			// Message should be accessed via
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
					"msg_full_name":  msg_full_name,
					"field_name":     f.GetName(),
				})
		} else {
			fmt.Fprintf(os.Stderr, "Unsupported to generate Reflection for field: %s, type: %s\n", f.GetName(), f.GetType())
		}
	}
	src += "    return " + singleton_name + ";\n"
	src += "}\n"

	fp.source_content += src
}
