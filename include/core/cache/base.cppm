export module core_cache;

// Public surface only. Storage partitions are re-exported for toolchain
// correctness but their declarations are not exported, so importers cannot
// name a backend, a driver or a connection. Entry point: CacheBuilder -> Cache.

export import :cache_value;
export import :cache_entry;
export import :cache_result;
export import :cache_stats;
export import :ttl;
export import :cache_engine;
export import :eviction_policy;
export import :cache_compare;

export import :string_literal;
export import :member_pointer_traits;
export import :cache_field;
export import :cache_entity_meta;
export import :key_builder;
export import :entity_codec;

export import :cache_predicate;
export import :cache_query_spec;
export import :cache_query;

export import :cache;
export import :cache_builder;

// Re-exported as interface partitions; contents are NOT exported.
export import :cache_storage_backend;
export import :local_cache_backend;
export import :cache_driver;
export import :cache_runtime;
export import :cache_context;
