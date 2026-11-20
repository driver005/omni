# Parity: revenue_recovery_redis

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/revenue_recovery_redis.rs`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:3542-3569`
- Upstream core: no `core/revenue_recovery_redis.rs`. Logic via `core/revenue_recovery_data_backfill.rs:118-155` → `types/storage/revenue_recovery_redis_operation.rs:1073-1146` (`RedisTokenManager::get_redis_key_data_raw`); types in `api_models/src/revenue_recovery_data_backfill.rs:204-220`
- C++: `include/app/handlers/revenue_recovery_redis.cppm`
- Checked: 2026-11-20 (re-verified after strict-parity port)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | GET `/v2/recovery/data-backfill/redis-data/{connector_cutomer_id}` (`app.rs:3546`, `:3554`, `:3568-3569`); C++ `Router{router, "/v2/recovery/data-backfill"}` + GET `/redis-data/:connector_cutomer_id` (`revenue_recovery_redis.cppm:141-157`). Both preserve the `cutomer` typo. |
| Input types | MATCH | Upstream `web::Path<String>` + `web::Query<GetRedisDataQuery>.key_type` enum `RedisKeyType{Status,Tokens}` (`revenue_recovery_redis.rs:15-20`, `api_models:204-213`); C++ reads the final path segment via `RequestPath`, rejects an empty id (`:61-70`) and strictly validates `key_type` (missing/invalid → `INVALID_REQUEST`, no defaulting) (`:72-95`). |
| Output types | MATCH | `RedisDataResponse{exists,ttl_seconds,data}` (`api_models:215-220`; DTO `app/dto/revenue_recovery_data_backfill.cppm:71-83`) emitted at `revenue_recovery_redis.cppm:132-138` from the ported tuple `(exists, ttl, data)` — exact wire shape and field order. |
| Auth/permissions | MATCH | Upstream `auth::V2AdminApiAuth` (`revenue_recovery_redis.rs:30`); C++ `AppPermission::ADMIN` via `auth_rules().rule_for("get_revenue_recovery_redis_data")` (`:45-56`). |
| Persistence | MATCH | Upstream real Redis `get_redis_conn` → `get_ttl` → key/hash read (`revenue_recovery_redis_operation.rs:1078-1145`). C++ ports the same algorithm over an in-process cache-backed Redis substitute (`revenue_recovery_redis.cppm:185-285`); the cache entry is the Redis key, expiry carries the TTL, and the raw string value carries the GET/hash payload. Real Redis is external infra, intentionally substituted locally per the strict-parity/local-substitute decision; the observable algorithm and wire result are identical. |
| Domain logic | MATCH | TTL semantics -2/-1/remaining (`:207-220`), Status `get_key::<String>` success/error shapes (`:222-242`), Tokens `get_hash_fields::<HashMap<String,String>>` existence/data shapes (`:244-272`), and `GenericNotFoundError` mapping on infrastructure failure (`:106-122`) all mirror upstream (`revenue_recovery_redis_operation.rs:1091-1145`, `core/revenue_recovery_data_backfill.rs:130-154`). |

## Routes
Upstream v2 scope `/v2/recovery` (`app.rs:3546`), `/data-backfill` scope (`app.rs:3554`), resource `/redis-data/{connector_cutomer_id}` GET (`app.rs:3568-3569`), gated `#[cfg(feature = "v2")]` (`app.rs:3537`). C++ `Router{router, "/v2/recovery/data-backfill"}` with GET `/redis-data/:connector_cutomer_id` (`revenue_recovery_redis.cppm:141-157`).

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/v2/recovery/data-backfill/redis-data/{connector_cutomer_id}` | `get_revenue_recovery_redis_data` (`revenue_recovery_redis.rs:11-34`) | `handle_get_revenue_recovery_redis_data` (`revenue_recovery_redis.cppm:39-139`) | Up: `Path<String>` id + `Query<GetRedisDataQuery>.key_type` (`revenue_recovery_redis.rs:15-20`, `api_models:204-213`); C++: final path segment + strict `key_type` validation, missing id/`key_type` → `INVALID_REQUEST` (`:61-95`) | Up: `ApplicationResponse<Json<RedisDataResponse{exists,ttl_seconds,data}>>` (`core/revenue_recovery_data_backfill.rs:130-144`); C++: ported tuple → `RedisDataResponse` (`:124-138`) | Up: `V2AdminApiAuth` (`revenue_recovery_redis.rs:30`); C++: `AppPermission::ADMIN` (`:45-56`) | Up: real Redis `get_ttl` + `get_key`/`get_hash_fields`, `GenericNotFoundError` only on Redis error (`revenue_recovery_redis_operation.rs:1073-1146`, `core:146-154`); C++: cache-backed Redis substitute with identical semantics (`:185-285`), 404 on closed cache (`:106-122`) | MATCH | Up `routes/revenue_recovery_redis.rs:11-34`, `app.rs:3546`/`:3554`/`:3568-3569`, `core/revenue_recovery_data_backfill.rs:118-155`, `types/storage/revenue_recovery_redis_operation.rs:1073-1146`, `api_models:204-220`, `redis_interface/src/errors.rs:20-21`, `redis_interface/src/module/redis_rs/commands.rs:752-762`/`:1093-1127`; C++ `revenue_recovery_redis.cppm:29-317` (helper `:185-273`, route `:141-157`). |

## Local Redis substitute contract
The C++ handler ports `get_redis_key_data_raw` verbatim but backs it with the injected `core::cache` instead of a Redis client (external infra, substituted locally by decision).

- Key: natural Redis key `customer:{id}:status` / `customer:{id}:tokens` (`revenue_recovery_redis_operation.rs:139-145`, `:1086-1089`), namespaced as `revenue_recovery_redis.redis:{key}` (`revenue_recovery_redis.cppm:102-104`).
- Status fallback: also reads `lock:revenue_recovery.status:{id}` (`app/locking/lock_registry.cppm:57-60`, `app/handlers/revenue_recovery_data_backfill.cppm:442-445`), whose plain-string value is the payment id — exactly upstream's Status value (`revenue_recovery_redis_operation.rs:162-168`).
- TTL: cache-entry expiry → Redis TTL `-2` (absent), `-1` (no expiry), else remaining whole seconds (`revenue_recovery_redis.cppm:207-220`; upstream `get_ttl` at `redis_interface/src/module/redis_rs/commands.rs:752-762`).
- Status miss: `exists=false`, `data="Error retrieving status key: Failed to get key value in Redis"` (`:235-240`), where the suffix is `RedisError::GetFailed`'s `Display` (`redis_interface/src/errors.rs:20-21`) and matches upstream's `format!("Error retrieving status key: {}", error)` (`revenue_recovery_redis_operation.rs:1104-1113`).
- Tokens miss: `exists=false`, `data={}` (`HGETALL` on a missing key → empty `HashMap` → `Ok(empty)` → `Value::Object({})`), `:245-255`; hash decode failure → `exists=false`, `data=null` (`:256-269`), matching upstream's `Err` branch (`revenue_recovery_redis_operation.rs:1129-1133`).
- Error: a closed cache is the local analog of `get_redis_conn`/`get_ttl` failure; it maps to the exact upstream `GenericNotFoundError` message (`:106-122`, `core/revenue_recovery_data_backfill.rs:146-154`). A missing key is NOT an error upstream and is returned as a 200 with `exists=false` (`:124-138`).

## Shared contract requests
None requiring edits to files outside this module. Two informal contracts are relied on locally and documented above so no shared code changes are needed: (1) the `revenue_recovery_redis.redis:*` cache namespace for raw Redis payloads, and (2) the producer's existing `lock:revenue_recovery.status:{id}` key, read only as a Status fallback. Neither requires changing `context/`, `services/`, `auth/`, `store/`, or other modules.

## Report
1. **Rows moved to MATCH** — the single route row `GET /v2/recovery/data-backfill/redis-data/{connector_cutomer_id}` (`get_revenue_recovery_redis_data`), from STUB to MATCH, plus all six summary dimensions (route paths/verbs, input types, output types, auth/permissions, persistence, domain logic) to MATCH. The upstream `get_redis_data` → `get_redis_key_data_raw` logic was ported line-by-line into the private local helper `get_redis_key_data_raw` (`revenue_recovery_redis.cppm:185-273`) over a `core::cache` Redis substitute; wire shapes, TTL semantics, Status/Tokens data shapes, auth, and the exact `GenericNotFoundError` message match upstream.
2. **Remaining non-MATCH** — none. Real Redis connectivity is intentionally replaced by the in-process `core::cache` substitute per the strict-parity/local-substitute decision (external infra). No C++-only routes exist in this module; the sole registered route has an upstream counterpart.
3. **Shared contract requests** — none (see section above); both relied-upon key contracts are local and documented.
4. **Files changed** — `include/app/handlers/revenue_recovery_redis.cppm` (handler + local helper port) and `docs/hyperswitch-parity/revenue_recovery_redis.md` (this report). No other files edited; `docs/hyperswitch/` left read-only. Bazel was not run.
