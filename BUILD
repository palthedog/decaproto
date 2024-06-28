package(default_visibility = ["//visibility:public"])

load("@bazel_gazelle//:def.bzl", "gazelle")

# gazelle:prefix github.com/palthedog/decaproto
gazelle(
    name = "gazelle",
    # Maintain BUILD only under generator/ directory
    args = ["generator"],
    command = "update",
)
