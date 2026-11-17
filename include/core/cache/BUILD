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
        "value.cppm",
        "entry.cppm",
        "result.cppm",
        "stats.cppm",
        "ttl.cppm",
        "engine.cppm",
        "eviction_policy.cppm",
        "compare.cppm",
        # compile-time reflection
        "string_literal.cppm",
        "member_pointer_traits.cppm",
        "field.cppm",
        "entity_meta.cppm",
        "key_builder.cppm",
        "entity_codec.cppm",
        # query support
        "predicate.cppm",
        "query_spec.cppm",
        "query.cppm",
        # hidden storage layer
        "driver.cppm",
        "runtime.cppm",
        "context.cppm",
        # facade + builder
        "cache.cppm",
        "builder.cppm",
        "aside.cppm",
    ],
    deps = [],

    visibility = ["//visibility:public"],
)
