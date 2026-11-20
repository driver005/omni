# Parity: cache

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/cache.rs:11-30`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2603-2610`
- Upstream core: `docs/hyperswitch/crates/router/src/core/cache.rs:8-25`
- Upstream pub/sub: `docs/hyperswitch/crates/storage_impl/src/redis/cache.rs:967-1012,372-421`; `docs/hyperswitch/crates/storage_impl/src/redis/pub_sub.rs:9-29,32-133`
- Upstream app wiring: `docs/hyperswitch/crates/router/src/routes/app.rs:555-597,639`; `docs/hyperswitch/crates/storage_impl/src/redis/cache.rs:23,190-193`
- Upstream error wire: `docs/hyperswitch/crates/api_models/src/errors/types.rs:60-70,89-118,159-176`; `docs/hyperswitch/crates/api_models/src/errors/actix.rs:3-31`; `docs/hyperswitch/crates/hyperswitch_domain_models/src/errors/api_error_response.rs:422-423,560-564`
- C++: `include/app/handlers/cache.cppm`
- Checked: 2026-11-20

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Both expose `POST /cache/invalidate/{key}`; C++ route `/invalidate/:key` (`cache.cppm:150-166`). |
| Input types | MATCH | Both read an untyped string key from the path; C++ uses `RequestPath` directly (`cache.cppm:126-128,280-293`), no synthetic JSON body. |
| Output types | MATCH | Success is bare empty-body `StatusOk` (`cache.cppm:145-147,337-349`); failures emit exact `ApiErrorResponse` wire bodies — InternalServerError `{"error":{"type":"api","message":"Something went wrong","code":"HE_00"}}` HTTP 500 (`cache.cppm:295-315`) and admin-auth Unauthorized `{"error":{"type":"invalid_request","message":"API key not provided or invalid API key used","code":"IR_01"}}` HTTP 401 (`cache.cppm:317-335`). |
| Auth/permissions | MATCH | Upstream `AdminApiAuth` admin-api-key check via `server_wrap` (`routes/cache.rs:20-28`, `authentication.rs:1713-1767`); C++ enforces the same PLATFORM_ADMIN+ADMIN+bearer requirement via registry row `invalidate` (`auth/registry.cppm:187`, `auth/route_auth.cppm:43,74-78`) and `authorizer().authorize_route` (`cache.cppm:112-124`). Flow instrumentation recorded (`cache.cppm:107-110`); `LockAction::NotApplicable` means no lock acquired (`cache.cppm:112-118`). |
| Persistence | MATCH | Full port of `redact_from_redis_and_publish` (`core/cache.rs:13`, `storage_impl/redis/cache.rs:967-1012`): DEL the key (`cache.cppm:202`), then PUBLISH `CacheRedact{tenant,kind:All(key)}` (`cache.cppm:226-238`) to the invalidation channel `hyperswitch_invalidate` (`cache.cppm:189-190`, upstream `cache.rs:23,190-193`). A process-wide in-process pub/sub broker (`cache.cppm:20-96`) fans out to every registered AppContext subscriber (`cache.cppm:168-186`); each subscriber runs the `on_message` invalidation (`pub_sub.rs:92-133`, `cache.cppm:240-278`). This is the local in-process substitute for Redis pub/sub and gives real cross-instance fan-out within the process. |
| Domain logic | MATCH | Upstream returns `StatusOk` iff published-channel count `> 0`, else InternalServerError (`core/cache.rs:17-24`); C++ gates on the broker's subscriber count `result == 0` (`cache.cppm:135-141`) with the same error body. A zero-delete Redis reply is never a failure (`cache.cppm:130-134`, upstream `cache.rs:988-998`). |

## Routes
Single endpoint. Upstream `invalidate` is wrapped by `server_wrap` with `Flow::CacheInvalidate`, `AdminApiAuth`, and `LockAction::NotApplicable` (`routes/cache.rs:16-28`). C++ records the flow metric, enforces the admin rule, and acquires no lock (`cache.cppm:107-124`).

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/cache/invalidate/{key}` | `cache::invalidate` (`core/cache.rs:8-25`) | `handle_invalidate` (`cache.cppm:101-148`) | Up: `web::Path<String>` key (`routes/cache.rs:14,18`); C++: path segment via `RequestPath` (`cache.cppm:126-128,280-293`), no JSON body | Up: `StatusOk` / `InternalServerError` (`core/cache.rs:19-24`); C++: empty-body `StatusOk` (`cache.cppm:145-147`) and exact `ApiErrorResponse` bodies (`cache.cppm:295-315,317-335`) | Up: `AdminApiAuth` (`routes/cache.rs:26`); C++: PLATFORM_ADMIN+ADMIN bearer rule via `auth_rules`/`authorizer` (`cache.cppm:112-124`) | Up: `redact_from_redis_and_publish`, `>0` channels required (`core/cache.rs:13-24`); C++: ported DEL+PUBLISH with in-process broker fan-out and `result==0` gate (`cache.cppm:135-141,192-207`) | MATCH | Up `routes/cache.rs:11-30`, `core/cache.rs:8-25`, `storage_impl/redis/cache.rs:967-1012,372-421`, `pub_sub.rs:9-133`, `app.rs:555-597,2606-2610`; C++ `cache.cppm:20-96,101-186,189-349`, route `:150-166` |

## Not ported
None. Every upstream behavior for this route is ported; external Redis pub/sub is reproduced by a local in-process broker (an allowed local substitute for external infra).

## Implementation disposition
Re-verified against current `include/app/handlers/cache.cppm`.

- Redis pub/sub broadcast of the invalidation — IMPLEMENTED (local substitute) — `InProcessPubSubBroker` (`cache.cppm:20-96`) models Redis PUBLISH/SUBSCRIBE: `publish()` fans out to every subscriber and returns the receiver count. Each AppContext registers a subscriber in the handler constructor (`cache.cppm:168-186`); the subscriber decodes `CacheRedact` and erases the key (`cache.cppm:240-278`), mirroring the process-wide `on_message` handler (`pub_sub.rs:92-133`). The payload is the exact serde wire of `CacheRedact{kind: All(key)}` (`cache.cppm:226-238`; upstream `cache.rs:372-392,413-421`) on channel `hyperswitch_invalidate` (`cache.cppm:189-190`; upstream `cache.rs:23,190-193`). The former durable-event-bus substitute was removed (upstream publishes only to Redis pub/sub).
- Success gating on published-channel count — IMPLEMENTED — `result == 0` (the broker's subscriber count) returns the exact InternalServerError body (`cache.cppm:135-141`); a cache miss (zero DEL) is never a failure (`cache.cppm:202`), matching upstream `core/cache.rs:17-24` and `cache.rs:988-998`.
- `server_wrap` middleware semantics — IMPLEMENTED (local substitute) — flow metric recorded up front (`cache.cppm:107-110`), auth via the admin registry rule (`cache.cppm:112-124`), no lock acquired (`LockAction::NotApplicable` equivalent, `cache.cppm:112-118`).
- Error shape parity — IMPLEMENTED — InternalServerError maps to `ApiError::new("HE",0,"Something went wrong")` → `{"error":{"type":"api","message":"Something went wrong","code":"HE_00"}}` HTTP 500 (`cache.cppm:295-315`; upstream `api_error_response.rs:422-423`, `types.rs:60-70,89-118`, `actix.rs:15`). The `attach_printable("Failed to invalidate cache")` is an attachment, not part of `current_context()`, so it is correctly absent from the body (`core/cache.rs:22-23`). Admin-auth failure maps to `Unauthorized` → `IR_01` HTTP 401 (`cache.cppm:317-335`; upstream `api_error_response.rs:560-564`, `types.rs:159-176`, `actix.rs:8`).
- Response shape parity — IMPLEMENTED — bare empty-body `StatusOk` (`cache.cppm:145-147,337-349`; upstream `core/cache.rs:20`, `services/api.rs:499`).
- Cross-tenant / cross-instance cache coherence — IMPLEMENTED (local in-process substitute) — a publish reaches every registered AppContext subscriber in the process (`cache.cppm:71-90,168-186`), each invalidating its own cache for `CacheKind::All(key)`. This is the in-process analogue of the shared process-wide `Caches` and single subscriber (`app.rs:555-597,639`; `pub_sub.rs:34-69,92-133`). Cross-process fan-out would require a real Redis broker and is intentionally out of scope (external infra).

## Shared contract requests
- **Auth error wire shape (cross-cutting):** `app::auth::authorizer().authorize_route` returns a local `AppError` (`{"error":"UNAUTHORIZED","message":...}`), not an upstream-shaped `ApiErrorResponse`. This handler maps its auth failure to the exact upstream `Unauthorized`/`IR_01` body locally (`cache.cppm:317-335`). A shared upstream-shaped error emitter in `app/auth` would let every module drop this per-handler mapping.
- **`server_wrap` equivalent (cross-cutting):** flow/telemetry/locking middleware is emulated per-handler with `metrics().record_flow` + `counter` and hand-applied auth. A shared middleware helper wrapping these facilities (AdminApiAuth + flow metric + LockAction) would remove the per-handler substitute without new infra.
- **Pub/sub broker (cross-cutting):** `InProcessPubSubBroker` lives in this module because no shared in-process channel abstraction exists in `app_context`/`core_events`. Promoting it to a shared facility would let other modules reproduce Redis pub/sub fan-out and channel-count success gating identically.
