export module core_database;

// Public surface only. The raw backend lives in the :driver and :context
// partitions, which are deliberately NOT re-exported here — a consumer that
// imports core_database cannot name, import, or reach the storage layer. They
// get the Database facade, the query builder, the reflection traits, and the
// value types. Nothing else.

export import :types;
export import :reflect;
export import :query;
export import :database;
