load("//bazel:build_defs.bzl", "congelado_module_library")

# core_bridge uses POCO (BCR) for the heavy lifting:
#   - Poco::Any               -> type-erased service registry (both directions)
#   - Poco::SharedLibrary     -> plugin .so loading + symbol resolution
# Poco is an external library; its headers are #included in the global module
# fragment of the relevant partitions, not imported as modules.

congelado_module_library(
    name = "core_bridge",
    primary_interface = "base.cppm",
    partitions = [
        "status.cppm",
        "service_key.cppm",
        "service_registry.cppm",
        "plugin_module.cppm",
        "runtime.cppm",
        "context.cppm",
        "bridge.cppm",
        "builder.cppm",
    ],
    deps = ["@poco//:PocoFoundation"],
    visibility = ["//visibility:public"],
)
