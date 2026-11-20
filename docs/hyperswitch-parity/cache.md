# Parity: cache

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/cache.rs:11-30`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2603-2610`
- Upstream core: `docs/hyperswitch/crates/router/src/core/cache.rs:8-25`
- C++: `include/app/handlers/cache.cppm`
- Checked: 2026-11-19

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Both expose `POST /cache/invalidate/{key}`; C++ route `/invalidate/:key` (`cache.cppm:103`). |
| Input types | MATCH | Both read an untyped string key from the path; C++ uses `RequestPath` directly, no synthetic JSON body (`cache.cppm:57,121-134`). |
| Output types | PARTIAL | Success is now bare empty-body `StatusOk` like upstream (`cache.cppm:94`); error body still uses local `AppError` JSON, not upstream `ApiErrorResponse` with change context (`cache.cppm:83-85`). |
| Auth/permissions | PARTIAL | Upstream `AdminApiAuth` via `server_wrap`; C++ resolves `auth_rules().rule_for("invalidate")` + `authorizer().authorize_route` (`cache.cppm:47-48`). Same intent, different mechanism. |
| Persistence | PARTIAL | Upstream `redact_from_redis_and_publish` (Redis pub/sub + shared `Caches`); C++ local `cache().erase(key)` plus durable event-bus publish as a local substitute; cross-process fan-out deferred (`cache.cppm:66,72-78`). |
| Domain logic | PARTIAL | C++ now publishes a typed `All(key)` envelope to local subscribers and gates success on event-id findability (`cache.cppm:72-85`); real Redis channel-count fan-out across instances remains a documented substitute/defer. |

## Routes
Single endpoint. Upstream `invalidate` is wrapped by `server_wrap` with `Flow::CacheInvalidate`, `AdminApiAuth`, and `LockAction::NotApplicable` (`routes/cache.rs:16-28`). C++ now records flow metrics (`cache.cppm:88-89`) and resolves auth via registry rules (`cache.cppm:47-48`); no lock is acquired, matching `NotApplicable`.

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/cache/invalidate/{key}` | `cache::invalidate` | `handle_invalidate` | Up: `web::Path<String>` key (`routes/cache.rs:14,18`); C++: path last segment via `RequestPath` (`cache.cppm:57,121-134`), no JSON body | Up: `StatusOk` / `InternalServerError` (`core/cache.rs:19-24`); C++: empty-body `StatusOk` on success, `AppError` JSON on publish failure (`cache.cppm:83-85,94`) | Up: `AdminApiAuth` (`routes/cache.rs:26`); C++: `auth_rules().rule_for("invalidate")` + `authorizer().authorize_route` (`cache.cppm:47-48`) | Up: `redact_from_redis_and_publish`, `>0` channels required (`core/cache.rs:13-24`); C++: local `cache().erase` + event-bus publish, success gated on event-id findability (`cache.cppm:66,72-85`) | PARTIAL | Up `routes/cache.rs:11-30`, `core/cache.rs:8-25`, `app.rs:2606-2610`; C++ `cache.cppm:37-95`, route `:101-113` |

## Not ported
- Redis pub/sub broadcast of the invalidation: upstream `redact_from_redis_and_publish(store, [CacheKind::All(key)])` publishes to all subscribed channels and invalidates the shared in-memory `Caches` (`core/cache.rs:3,13-15`; shared caches wired in `app.rs:566,575-576,691-692`). C++ only erases a local entry (`cache.cppm:63`), with an explicit comment that the pub/sub channel count has no local analogue (`cache.cppm:61-62`).
- Success gating on published-channel count — upstream returns `InternalServerError` when `result == 0` ("Failed to invalidate cache") (`core/cache.rs:19-24`). C++ never fails on a miss; it returns `removed:false` with status 200 (`cache.cppm:65-68`).
- `server_wrap` middleware semantics: `Flow::CacheInvalidate` instrumentation/metrics, `AdminApiAuth` request handling, and `LockAction::NotApplicable` (`routes/cache.rs:16-28`). C++ manually calls the authorizer (`cache.cppm:27-34`); no flow/metrics/lock layer.
- Error shape parity: upstream maps storage failures through `ApiErrorResponse::InternalServerError` with a change context (`core/cache.rs:15`); C++ uses `AppErrorBuilder` with `EMPTY_REQUEST`/`INVALID_JSON` kinds (`cache.cppm:45,56`) that upstream does not emit for this route.
- Response shape parity: upstream plain `StatusOk` has no body fields (`core/cache.rs:20`); C++ adds `key` and `removed` fields not present upstream (`cache.cppm:65-67`).
- Cross-tenant / cross-instance cache coherence: upstream `CacheKind::All(key)` plus shared `Caches` invalidates every tenant's store and every subscriber (`app.rs:419-421,565-576`); C++ `cache().erase(key)` affects only the local `AppContext` cache (`cache.cppm:63`).

## Implementation disposition
Re-verified against current `include/app/handlers/cache.cppm` (handler substantially reworked since the 2026-11-19 check).

- Redis pub/sub broadcast of the invalidation — SUBSTITUTED — C++ publishes a typed `CacheInvalidateEvent` (`kind="All"`, `key`) to `events().publish("cache.invalidate", …)` for the local subscriber set (`cache.cppm:72-78`); a real cross-process broker is still external.
- Success gating on published-channel count — SUBSTITUTED — C++ gates on `event_id.empty() || !find_by_event_id(event_id)` and returns INTERNAL_ERROR "Failed to invalidate cache" (`cache.cppm:83-85`); durable-record lookup stands in for upstream's `result == 0` channel count.
- `server_wrap` middleware semantics — SUBSTITUTED — flow instrumentation present via `metrics().record_flow("CacheInvalidate")` + `counter("CACHE_INVALIDATED")` (`cache.cppm:88-89`), auth via `auth_rules`/`authorizer` (`cache.cppm:47-48`); no lock acquired, matching `LockAction::NotApplicable`.
- Error shape parity — SUBSTITUTED — C++ now emits `INVALID_REQUEST` for empty key (`cache.cppm:59`) and `INTERNAL_ERROR` on publish failure (`cache.cppm:84`); upstream `ApiErrorResponse::InternalServerError` with a change context is not reproduced.
- Response shape parity — IMPLEMENTED — C++ sends bare empty-body `StatusOk` via `send_json(..., std::string_view{}, true)` (`cache.cppm:94`); no `key`/`removed` fields remain.
- Cross-tenant / cross-instance cache coherence — DEFERRED — requires a real Redis pub/sub broker and shared `Caches` (`app.rs:419-421,565-576`); C++ only erases the local entry (`cache.cppm:66`) and publishes to a local subscriber set (`cache.cppm:72-78`).
