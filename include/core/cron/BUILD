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
        "cron_engine.cppm",
        "cron_field.cppm",
        "cron_expression.cppm",
        "cron_job.cppm",
        "fire_event.cppm",
        # compile-time reflection
        "cron_job_meta.cppm",
        # query support
        "cron_predicate.cppm",
        "cron_query_spec.cppm",
        "cron_query.cppm",
        # hidden engine layer
        "cron_backend.cppm",
        "local_cron_backend.cppm",
        "cron_driver.cppm",
        "cron_runtime.cppm",
        "cron_context.cppm",
        # facade + builder
        "cron.cppm",
        "cron_builder.cppm",
    ],
    visibility = ["//visibility:public"],
)
