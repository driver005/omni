load("//bazel:build_defs.bzl", "congelado_module_library")

# core_cron mirrors core_router / core_database / core_cache: one primary
# interface, many partitions. Internal storage partitions are interface
# partitions for the toolchain but their declarations are not exported, so the
# Cron facade is the only public way to reach the timing engine.

congelado_module_library(
    name = "core_cron",
    primary_interface = "base.cppm",
    partitions = [
        # value types
        "string_literal.cppm",
        "engine.cppm",
        "field.cppm",
        "expression.cppm",
        "job.cppm",
        "fire_event.cppm",
        # compile-time reflection
        "job_meta.cppm",
        # query support
        "predicate.cppm",
        "query_spec.cppm",
        "query.cppm",
        # hidden engine layer
        "driver.cppm",
        "runtime.cppm",
        "context.cppm",
        # facade + builder
        "cron.cppm",
        "builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
