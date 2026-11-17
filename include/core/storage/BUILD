load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_storage",
    primary_interface = "base.cppm",
    partitions = [
        "storage_ref.cppm",
        "storage_result.cppm",
        "storage_backend.cppm",
        "memory_storage_backend.cppm",
        "storage_runtime.cppm",
        "storage.cppm",
        "storage_builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
