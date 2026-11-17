export module core_cache;

// Public surface only. Storage partitions are re-exported for toolchain
// correctness but their declarations are not exported, so importers cannot
// name a backend, a driver or a connection. Entry point: CacheBuilder -> Cache.

export import :value;
export import :entry;
export import :result;
export import :stats;
export import :ttl;
export import :engine;
export import :eviction_policy;
export import :compare;

export import :string_literal;
export import :member_pointer_traits;
export import :field;
export import :entity_meta;
export import :key_builder;
export import :entity_codec;

export import :predicate;
export import :query_spec;
export import :query;

export import :cache;
export import :builder;
export import :aside;

// Re-exported as interface partitions; contents are NOT exported.
export import :driver;
export import :runtime;
export import :context;
