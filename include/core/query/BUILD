load("//bazel:build_defs.bzl", "congelado_cc_test", "congelado_module_library")

# core_query: Ghost NQL / input pipeline. Self-contained (std only); consumes a
# JSON array string and applies filter / order / fields / limit / page / offset.
congelado_module_library(
    name = "core_query",
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
)

congelado_cc_test(
    name = "core_query",
    srcs = ["test.cc"],
    deps = [":core_query"],
)
