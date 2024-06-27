load("@rules_proto//proto:defs.bzl", "proto_library")
load("//generator:decaproto.bzl", "deca_proto_library")

proto_library(
    name = "simple_proto",
    srcs = ["simple.proto"],
)

deca_proto_library(
    name = "simple_deca_proto",
    protos = [":simple_proto"],
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
