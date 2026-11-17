export module core_database;

// Public surface. The storage layer lives in the :storage_backend,
// :in_memory_backend, :driver, :database_runtime and :db_context partitions.
// Those are deliberately NOT re-exported here, so an importer of core_database
// can never name or reach a connection, a driver or a backend class — the only
// way in is DatabaseBuilder -> Database.

export import :db_value;
export import :column_type;
export import :field_flags;
export import :compare;
export import :sort_order;
export import :storage_engine;
export import :row;
export import :query_result;

export import :string_literal;
export import :member_pointer_traits;
export import :field;
export import :entity_meta;
export import :sql_generator;
export import :entity_mapper;

export import :predicate;
export import :query_spec;
export import :query;

export import :database;
export import :database_builder;

// Interface partitions must be re-exported by the primary interface for the
// toolchain to accept them. Their contents are NOT exported (no `export` on
// the namespace), so a consumer importing core_database cannot name any of
// the backend classes below — they are reachable only inside this module.
export import :storage_backend;
export import :in_memory_backend;
export import :driver;
export import :database_runtime;
export import :db_context;
