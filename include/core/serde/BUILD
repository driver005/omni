load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "core_serde",
    primary_interface = "serde.cppm",
    partitions = [
        "core.cppm",
        "json_codec.cppm",
        "converter.cppm",
        "cache.cppm",
        "jsonld.cppm",
        "error_kind.cppm",
        "error.cppm",
        "serializer.cppm",
    ],
    deps = [
        "//include/interfaces:interfaces",
        "//include/io/layer/http2:io_layer_http2",
        "@reflect_cpp//:reflect-cpp",  # rfl.hpp / rfl/*.hpp
        "@simdjson//:simdjson",  # converter.cppm: simdjson.h
        "//third_party/tomlplusplus_shim:toml++",  # converter.cppm: toml++/toml.hpp
        "@stduuid//:stduuid",  # core.cppm / converter.cppm: uuid.h
    ],
    visibility = ["//visibility:public"],
)
