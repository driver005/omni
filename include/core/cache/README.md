# core_cache

C++20 module cache abstraction, built after the same pattern as
`include/core/router` and `include/core/database`. The backend, connection and
driver are **not part of the public interface** — callers go through
`CacheBuilder` → `Cache` and never receive anything that talks to storage
directly.

## Shape

`base.cppm` is the primary interface (`export module core_cache;`). Internal
storage partitions are re-exported only so the toolchain accepts them as
interface partitions; their declarations live in a non-exported namespace, so
an importer cannot name a backend or driver. `Cache` owns its state through a
private incomplete `Runtime` type (pImpl).

Each partition defines one class (or one cohesive value type) and one
`boost.ut` test suite under `CONGELADO_TEST`.

## Compile-time reflection

Plain structs become cache entities with a macro. Member names, pointers and
roles (KEY / TAG / PAYLOAD) are compile-time constants used to generate typed
keys, tags and codecs — no runtime reflection.

```cpp
import core_cache;

struct User {
    std::int64_t id{};
    std::string  name{};
    bool         admin{};
};

CORE_CACHE_ENTITY(User, "users",
    CORE_CACHE_FIELD("id",    &User::id,    core::cache::FieldRole::KEY),
    CORE_CACHE_FIELD("name",  &User::name,  core::cache::FieldRole::TAG),
    CORE_CACHE_FIELD("admin", &User::admin));
```

The macro emits an ADL hook (`core_cache_reflect_meta(User*)`), so it works at
any namespace scope.

## Queries and typed operations

```cpp
core::cache::CacheBuilder builder;
auto cache = builder.engine(core::cache::CacheEngine::LOCAL)
                  .capacity(1024)
                  .policy(core::cache::EvictionPolicy::LRU)
                  .build();                 // opaque handle; no backend exposed

User user{7, "Ada", true};
cache.put<User>(user, core::cache::Ttl::seconds(60));   // key "users:v1:7"
auto hit = cache.get<User>(user);

auto q = cache.query<User>();               // compile-time member checks
q.where<&User::id>(core::cache::CacheCompare::GE, "1")
 .with_tag("Ada")
 .limit(10);
auto entries = cache.query<User>(q.spec()); // typed results, decoded via codec

cache.erase<User>(user);
```

Raw operations remain available but return plain data (`CacheResult`,
`std::vector<CacheEntry>`), never a backend handle.

## Build

`BUILD` uses `congelado_module_library` with `base.cppm` as the primary
interface, mirroring `include/core/router/BUILD`.
