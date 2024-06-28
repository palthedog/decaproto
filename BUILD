package(default_visibility = ["//visibility:public"])

load("@bazel_gazelle//:def.bzl", "gazelle")

# gazelle:prefix github.com/palthedog/decaproto
gazelle(
    name = "gazelle",
    # Maintain BUILD only under generator/ directory
    command = "update",
)

gazelle(
    name = "gazelle-update-repos",
    args = [
        "golang.org/x/exp@v0.0.0-20240613232115-7f521ea00fb8",
    ],
    command = "update-repos",
)
