# Code style memory (this workspace)

Persistent summary of the user's preferred C++ style. Mirrors the rules appended to
`/home/default/dots/AGENTS.md` so they load every session.

## Construction / initialization (STRICT)

- **Always brace-initialize objects, never parentheses.**
  - Use `Cron{CronEngine::LOCAL}`.
  - Never `Cron(CronEngine::LOCAL)`.
  - Applies to every class construction, temporaries, members, and `make_unique` args
    where a choice exists: `Ttl{}.seconds(60)`, `ServiceKey{}.make<T>("x")`,
    `CronRuntime{CronEngine::LOCAL}`.
- Rationale: uniform init everywhere, avoids most-vexing-parse and narrowing surprises.

## Module architecture

- One C++20 module per subsystem: `base.cppm` primary interface (`export module core_x;`),
  partitions via `export module core_x:part;`.
- `base.cppm` re-exports public partitions; internal partitions re-exported only for
  toolchain correctness, with declarations in a non-exported namespace so consumers
  cannot name backend/driver types.
- Facade owns state via pImpl. Use `std::shared_ptr<detail::Runtime>` with an exported
  incomplete forward declaration so **all members can be defined inline** — no
  second namespace block, no out-of-line member definitions, no free helper functions.
- Builder is the only public entry point: `XBuilder` -> opaque `X` handle.

## Functions (STRICT)

- **No free functions.** Convert helpers to instance functors (class with `operator()`),
  e.g. `SqlTypeName{}`, `CompareValues{}`, `IsMiss{}`, `CacheEngineName{}`.
- **No static member functions.** Convert to non-static instance methods; call on a
  temporary where needed: `Ttl{}.seconds(60)`, `CronExpression{}.parse(expr)`,
  `SqlGenerator{}.select_columns<E>()`. Add a public default ctor to classes whose
  former static factories are now instance methods.
- Allowed exception: `operator|` / `operator&` on enums (no object to own them). Even
  these should be replaced with named methods (`combine()`, `intersects()`) if asked.
- No `consteval`/`static_assert` helper functions; compile-time metadata via ADL hooks
  emitted by macros (`CORE_DB_REFLECT`, `CORE_CACHE_ENTITY`, `CORE_CRON_JOB`).

## Files / tests

- One class (or one cohesive value type) and **one** `boost.ut` test suite per file.
- Tests live under `#ifdef CONGELADO_TEST` with `import boost.ut;` at the top.
- Each partition has its own `BUILD` using `congelado_module_library` with
  `base.cppm` as `primary_interface`.

## Reflection / queries

- Entity/job classes stay plain structs; macro emits ADL metadata hook. Names, member
  pointers, field roles, expressions are compile-time constants.
- Query builders are fluent and typed; member pointers validated with `static_assert`.

## Build

- Bazel only. No xmake, no conan.
- Online BCR deps in `MODULE.bazel`; `http_archive` only when no BCR module exists.
- `Makefile` wraps bazel: `make build`, `make dev`, `make test`, `make core`.
- External libs do the heavy lifting (POCO for `core_bridge`, hiredis/libpq for plugins),
  not hand-written platform code.

## Plugins

- Only two kept: `plugins/database/postgres_backend`, `plugins/cache/redis_backend`.
- Each implements `interfaces::IDatabase` / `ICache` and exposes
  `extern "C" void congelado_bridge_register(void*)` receiving the public `Bridge*`,
  calling `bridge->register_service(ServiceKey<T>::make<T>("name"), ptr)`.
