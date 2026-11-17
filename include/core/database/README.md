# core_database

C++20 module database abstraction, built after the same pattern as
`include/core/router`. The backend, connection and driver are **not part of the
public interface** — callers go through `DatabaseBuilder` → `Database` and
never receive anything that can be used to talk to storage directly.

## Shape

`base.cppm` is the primary interface (`export module core_database;`). It
re-exports the public partitions (value types, reflection, queries, facade).
The storage partitions (`storage_backend`, `in_memory_backend`, `driver`,
`database_runtime`, `db_context`) are re-exported only so the toolchain accepts
them as interface partitions; their declarations live in a non-exported
namespace, so a consumer importing `core_database` cannot name them at all.
`Database` owns its state through a private incomplete `Runtime` type (pImpl).

Each partition file defines one class (or one cohesive value type) and one
`boost.ut` test suite under `CONGELADO_TEST`.

## Compile-time reflection

Plain structs become entities with a macro. Column names, member pointers, SQL
types and flags are all compile-time constants; DDL, projections and row mapping
are generated from them (no runtime `type_info`, no string parsing).

```cpp
import core_database;

struct User {
    std::int64_t id{};
    std::string  name{};
    std::int64_t age{};
};

CORE_DB_REFLECT(User, "users",
    CORE_DB_FIELD("id",   &User::id,   core::db::ColumnType::BIGINT,
                  core::db::FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("name", &User::name, core::db::ColumnType::TEXT,
                  core::db::FieldFlags::NOT_NULL),
    CORE_DB_FIELD("age",  &User::age,  core::db::ColumnType::INTEGER));
```

The macro emits an ADL hook (`core_db_reflect_meta(User*)`), so it works at any
namespace scope — no explicit specialization across module boundaries.

## Queries and CRUD

```cpp
core::db::DatabaseBuilder builder;
auto db = builder
    .engine(core::db::StorageEngine::IN_MEMORY)
    .register_entity<User>()
    .build();                       // opaque handle; no connection is exposed

User saved = db.insert(User{0, "Ada", 36});   // generated primary key written back

auto q = db.query<User>();
q.where<&User::age>(core::db::Compare::GE, core::db::to_db_value(18))
 .order_by<&User::name>(core::db::SortOrder::ASC)
 .limit(10);
auto rows = db.execute(q.spec());             // QueryResult — plain data

auto one   = db.find<User>(core::db::to_db_value(saved.id));
auto all   = db.all<User>();
auto n     = db.count<User>();
db.remove<User>(core::db::to_db_value(saved.id));
```

Member pointers passed to `where`, `select`, `order_by`, `find_first_by` and
`update_where` are checked at compile time: a member that is not reflected on
the entity fails to compile.

## Build

`BUILD` uses `congelado_module_library` with `base.cppm` as the primary
interface, mirroring `include/core/router/BUILD`.
