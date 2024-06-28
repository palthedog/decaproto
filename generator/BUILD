load("@rules_proto_grpc//:defs.bzl", "proto_plugin")
load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

package(default_visibility = ["//visibility:public"])

go_binary(
    name = "decaproto",
    embed = [":generator_lib"],
    visibility = ["//visibility:public"],
)

go_library(
    name = "generator_lib",
    srcs = [
        "field.go",
        "main.go",
    ],
    importpath = "github.com/palthedog/decaproto/generator",
    visibility = ["//visibility:private"],
    deps = [
        "@com_github_golang_protobuf//protoc-gen-go/descriptor",
        "@com_github_golang_protobuf//protoc-gen-go/plugin",
        "@org_golang_google_protobuf//encoding/prototext:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
        "@org_golang_x_exp//maps:go_default_library",
    ],
)

proto_plugin(
    name = "decaproto_plugin",
    output_directory = True,
    tool = "//generator:decaproto",
)
