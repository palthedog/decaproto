cc_library(
    name = "decaproto",
    srcs = [
        "decoder.cc",
        "encoder.cc",
    ],
    hdrs = [
        "decoder.h",
        "encoder.h"
        "descriptor.h",
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
