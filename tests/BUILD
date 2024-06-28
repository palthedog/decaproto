load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("//generator:decaproto.bzl", "deca_proto_library")

proto_library(
    name = "test_proto",
    srcs = glob(["*.proto"]),
)

deca_proto_library(
    name = "test_deca_proto",
    protos = [":test_proto"],
)

cc_test(
    name = "simple_test",
    size = "small",
    srcs = ["simple_test.cc"],
    deps = [
        ":test_deca_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "repeated_test",
    size = "small",
    srcs = ["repeated_test.cc"],
    deps = [
        ":test_deca_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "def_order_test",
    size = "small",
    srcs = ["def_order_test.cc"],
    deps = [
        ":test_deca_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

proto_library(
    name = "tests_proto",
    srcs = [
        "def_order.proto",
        "nested.proto",
        "repeated.proto",
        "simple.proto",
    ],
    visibility = ["//visibility:public"],
)

go_proto_library(
    name = "tests_go_proto",
    importpath = "github.com/palthedog/decaproto/tests",
    proto = ":tests_proto",
    visibility = ["//visibility:public"],
)

go_library(
    name = "tests",
    embed = [":tests_go_proto"],
    importpath = "github.com/palthedog/decaproto/tests",
    visibility = ["//visibility:public"],
)
