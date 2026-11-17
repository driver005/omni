load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_email",
    primary_interface = "base.cppm",
    partitions = [
        "address.cppm",
        "message.cppm",
        "template_engine.cppm",
        "provider.cppm",
        "runtime.cppm",
        "email.cppm",
        "builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
