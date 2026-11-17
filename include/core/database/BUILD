load("//bazel:build_defs.bzl", "congelado_module_library")

# core_database mirrors the core_router module layout: one primary interface,
# many partitions. Internal partitions (storage_backend, in_memory_backend,
# driver, database_runtime, db_context) are listed as partitions but are NOT
# re-exported by base.cppm — the Database facade is the only public entry point.

congelado_module_library(
    name = "core_database",
    primary_interface = "base.cppm",
    partitions = [
        "storage_backend.cppm",
        # value types
        "db_value.cppm",
        "column_type.cppm",
        "field_flags.cppm",
        "compare.cppm",
        "sort_order.cppm",
        "storage_engine.cppm",
        "row.cppm",
        "query_result.cppm",
        # compile-time reflection
        "string_literal.cppm",
        "member_pointer_traits.cppm",
        "field.cppm",
        "entity_meta.cppm",
        "sql_generator.cppm",
        "entity_mapper.cppm",
        # query support
        "predicate.cppm",
        "query_spec.cppm",
        "query.cppm",
        # hidden storage layer
        "driver.cppm",
        "runtime.cppm",
        "db_context.cppm",
        # facade + builder
        "database.cppm",
        "builder.cppm",
        "migration.cppm",
        "migration_runner.cppm",
        "seeder.cppm",
        "relation.cppm",
        "repository_base.cppm",
    ],
    deps = [
        "//include/core/serde:core_serde",],

    visibility = ["//visibility:public"],
)
