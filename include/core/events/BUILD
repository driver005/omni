load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_events",
    primary_interface = "events.cppm",
    partitions = [
        "registry.cppm",
        "webhook.cppm",
        "webhook_dispatcher.cppm",
        "stats_counter.cppm",
    ],
    deps = [
        "//include/interfaces:interfaces",
        "//include/core/logger:core_logger",
    ],
    visibility = ["//visibility:public"],
)
