load("@rules_cc//cc:defs.bzl", "cc_library")
load(
    "@rules_proto_grpc//:defs.bzl",
    "ProtoPluginInfo",
    "proto_compile_attrs",
    "proto_compile_impl",
)

# https://rules-proto-grpc.com/en/latest/custom_plugins.html
deca_proto_compile = rule(
    implementation = proto_compile_impl,
    attrs = dict(
        proto_compile_attrs,
        _plugins = attr.label_list(
            providers = [ProtoPluginInfo],
            default = [
                Label("//codegen:decaproto_plugin"),
            ],
            cfg = "exec",
            doc = "List of protoc plugins to apply",
        ),
    ),
    toolchains = [str(Label("@rules_proto_grpc//protoc:toolchain_type"))],
)

def deca_proto_library(name, protos, deps = []):
    compiled_name = name + "_comp"
    deca_proto_compile(
        name = compiled_name,
        protos = protos,
    )

    cc_library(
        name = name,
        srcs = [compiled_name],
        hdrs = [compiled_name],
        deps = deps + [
            "//runtime/decaproto",
        ],
        linkstatic = True,
        includes = [compiled_name],
    )
