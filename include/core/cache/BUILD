load("//bazel:build_defs.bzl", "congelado_module_library")

# core_cache mirrors core_router / core_database: one primary interface, many
# partitions. Internal storage partitions are interface partitions for the
# toolchain but their declarations are not exported, so the Cache facade is the
# only public way to reach storage.

congelado_module_library(
    name = "core_cache",
    primary_interface = "base.cppm",
    partitions = [
        # value types
        "cache_value.cppm",
        "cache_entry.cppm",
        "cache_result.cppm",
        "cache_stats.cppm",
        "ttl.cppm",
        "cache_engine.cppm",
        "eviction_policy.cppm",
        "cache_compare.cppm",
        # compile-time reflection
        "string_literal.cppm",
        "member_pointer_traits.cppm",
        "cache_field.cppm",
        "cache_entity_meta.cppm",
        "key_builder.cppm",
        "entity_codec.cppm",
        # query support
        "cache_predicate.cppm",
        "cache_query_spec.cppm",
        "cache_query.cppm",
        # hidden storage layer
        "cache_storage_backend.cppm",
        "local_cache_backend.cppm",
        "cache_driver.cppm",
        "cache_runtime.cppm",
        "cache_context.cppm",
        # facade + builder
        "cache.cppm",
        "cache_builder.cppm",
        "cache_aside.cppm",
    ],
    deps = [],

    visibility = ["//visibility:public"],
)
