load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_search",
    primary_interface = "base.cppm",
    partitions = [
        "backend.cppm",
        "query.cppm",
        "hit.cppm",
        "result.cppm",
        "runtime.cppm",
        "search.cppm",
        "builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
