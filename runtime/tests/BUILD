cc_library(
    name = "fake_message",
    srcs = ["fake_message.cc"],
    hdrs = ["fake_message.h"],
    visibility = ["//visibility:private"],
    deps = [
        "//runtime/decaproto",
        "//runtime/decaproto/stream",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "coded_stream_test",
    size = "small",
    srcs = ["coded_stream_test.cc"],
    deps = [
        "//runtime/decaproto/stream",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "reflection_test",
    size = "small",
    srcs = ["reflection_test.cc"],
    deps = [
        ":fake_message",
        "//runtime/decaproto",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "decoder_test",
    size = "small",
    srcs = ["decoder_test.cc"],
    deps = [
        ":fake_message",
        "//runtime/decaproto",
        "//runtime/decaproto/stream",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "encoder_test",
    size = "small",
    srcs = ["encoder_test.cc"],
    deps = [
        ":fake_message",
        "//runtime/decaproto",
        "//runtime/decaproto/stream",
        "@googletest//:gtest_main",
    ],
)
