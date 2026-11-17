load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_storage",
    primary_interface = "base.cppm",
    partitions = [
        "backend.cppm",
        "ref.cppm",
        "result.cppm",
        "runtime.cppm",
        "storage.cppm",
        "builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
