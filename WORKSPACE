workspace(name = "decaproto")

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")
load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", "go_repository")

go_repository(
    name = "org_golang_x_exp",
    importpath = "golang.org/x/exp",
    sum = "h1:yixxcjnhBmY0nkL253HFVIm0JsFHwrHdT3Yh6szTnfY=",
    version = "v0.0.0-20240613232115-7f521ea00fb8",
)

go_rules_dependencies()
go_register_toolchains(version = "1.20.7")
gazelle_dependencies()
