load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_settings",
    primary_interface = "base.cppm",
    partitions = [
        "setting_value.cppm",
        "setting_key.cppm",
        "settings_validator.cppm",
        "settings_map.cppm",
        "settings_runtime.cppm",
        "settings.cppm",
        "settings_builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
