load("//bazel:build.bzl", "redpanda_cc_library")

redpanda_cc_library(
    name = "stm",
    srcs = [
        "transform_offsets_stm.cc",
    ],
    hdrs = [
        "include/transform/stm/transform_offsets_stm.h",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    deps = [
        "//src/v/cluster",
        "//src/v/model",
        "@seastar",
    ],
)
