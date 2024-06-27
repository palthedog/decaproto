load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")
load("//generator:decaproto.bzl", "deca_proto_library", "deca_proto_compile")

proto_library(
    name = "simple_proto",
    srcs = ["simple.proto"],
)

deca_proto_library(
    name = "simple_deca_proto",
    protos = [":simple_proto"],
)

cc_library(
    name = "foo",
    hdrs = ["simple_test.h"],
)

cc_test(
    name = "simple_test",
    size = "small",
    srcs = ["simple_test.cc"],
    deps = [
        ":simple_deca_proto",
        "@com_google_googletest//:gtest_main",
    ],
)
