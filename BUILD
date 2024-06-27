package(default_visibility = ["//visibility:public"])

load("@bazel_gazelle//:def.bzl", "gazelle")

# gazelle:prefix github.com/palthedog/decaproto
gazelle(name = "gazelle")

proto_library(
    name = "simple_proto",
    srcs = glob(["generator/simple.proto"]),
)
