cc_library(
    name = "decaproto",
    srcs = [
        "decoder.cc",
        "encoder.cc",
    ],
    hdrs = [
        "decoder.h",
        "descriptor.h",
        "encoder.h",
        "field.h",
        "message.h",
        "reflection.h",
        "reflection_util.h",
    ],
    strip_include_prefix = "/runtime",
    visibility = ["//visibility:public"],
    deps = [
        "//runtime/decaproto/stream",
    ],
)
