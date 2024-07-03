load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")
load("@rules_proto_grpc//:defs.bzl", "proto_plugin")

package(default_visibility = ["//visibility:public"])

go_binary(
    name = "decaproto",
    embed = [":codegen_lib"],
    visibility = ["//visibility:public"],
)

go_library(
    name = "codegen_lib",
    srcs = [
        "descriptor.go",
        "field.go",
        "main.go",
        "reflection.go",
        "template.go",
    ],
    importpath = "github.com/palthedog/decaproto/codegen",
    visibility = ["//visibility:private"],
    deps = [
        "@com_github_golang_protobuf//protoc-gen-go/descriptor",
        "@com_github_golang_protobuf//protoc-gen-go/plugin",
        "@org_golang_google_protobuf//encoding/prototext",
        "@org_golang_google_protobuf//proto",
        "@org_golang_x_exp//maps:go_default_library",
        "@org_golang_x_text//cases",
        "@org_golang_x_text//language",
    ],
)

proto_plugin(
    name = "decaproto_plugin",
    output_directory = True,
    tool = "//codegen:decaproto",
)
