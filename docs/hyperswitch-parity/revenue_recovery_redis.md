# Parity: revenue_recovery_redis

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/revenue_recovery_redis.rs`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:3542-3569`
- Upstream core: no `core/revenue_recovery_redis.rs`. Logic via `core/revenue_recovery_data_backfill.rs:118-155` → `types/storage/revenue_recovery_redis_operation.rs:1073-1145` (`RedisTokenManager::get_redis_key_data_raw`); types in `api_models/src/revenue_recovery_data_backfill.rs:204-220`
- C++: `include/app/handlers/revenue_recovery_redis.cppm`
- Checked: 2026-11-19

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | GET `/v2/recovery/data-backfill/redis-data/{connector_cutomer_id}` upstream (scope `app.rs:3546`, `/data-backfill` `:3554`, resource `:3568-3569`); C++ `Router{router, "/v2/recovery/data-backfill"}` + GET `/redis-data/:connector_cutomer_id` (`revenue_recovery_redis.cppm:101-112`). Both preserve upstream `cutomer` typo. Upstream v2-gated (`app.rs:3537`); C++ always registers. |
| Input types | MATCH | Upstream `web::Path<String>` + `web::Query<GetRedisDataQuery>` enum `RedisKeyType{Status,Tokens}` (`revenue_recovery_redis.rs:15-20`, `api_models:204-213`). C++ reads final path segment via `RequestPath`, rejects empty id (`revenue_recovery_redis.cppm:45-54`), and strictly validates `key_type`: missing/invalid → `INVALID_REQUEST` instead of defaulting to Status (`revenue_recovery_redis.cppm:60-79`). Manual parsing, but observable input behavior matches serde. |
| Output types | PARTIAL | Same `RedisDataResponse{exists,ttl_seconds,data}` shape (`api_models:215-220`, DTO `app/dto/revenue_recovery_data_backfill.cppm:71-89`). C++ no longer hardcodes: `exists` derived from decoded data (`revenue_recovery_redis.cppm:150-160`), `ttl_seconds` from cache entry expiry (`:138-148`), `data` typed JSON decode of cache payload (`:116-129`), emitted at `:166-173`. Values remain cache-backed stand-ins, not real Redis — PARTIAL. |
| Auth/permissions | MATCH | Upstream `auth::V2AdminApiAuth` (`revenue_recovery_redis.rs:30`); C++ `AppPermission::ADMIN` token check via `auth_rules().rule_for("get_revenue_recovery_redis_data")` (`revenue_recovery_redis.cppm:29-40`). |
| Persistence | STUB | Upstream real Redis: `get_redis_conn` → `get_ttl` → key/hash read (`revenue_recovery_redis_operation.rs:1078-1135`). C++ read-only in-memory `cache().execute` of producer-populated stand-in payload, no Redis client (`revenue_recovery_redis.cppm:98-114`). Real Redis is external dependency — STUB retained. |
| Domain logic | PARTIAL | Upstream TTL retrieval + key-type-specific reads + `GenericNotFoundError` (`revenue_recovery_redis_operation.rs:1086-1135`, `core:130-154`). C++ now builds key-type natural keys (`revenue_recovery_redis.cppm:81-88`), derives TTL from cache expiry (`:138-148`), typed Status/Tokens decode + data-driven `exists` (`:116-160`), and upstream error message (`:90-93`, `:108-114`) — all against a cache stand-in, not Redis. |

## Routes
Upstream v2 scope `/v2/recovery` (`app.rs:3546`), `/data-backfill` scope (`app.rs:3554`), resource `/redis-data/{connector_cutomer_id}` GET (`app.rs:3568-3569`), gated `#[cfg(feature = "v2")]` (`app.rs:3537`). C++ `Router{router, "/v2/recovery/data-backfill"}` with GET `/redis-data/:connector_cutomer_id` (`revenue_recovery_redis.cppm:101-112`).

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/v2/recovery/data-backfill/redis-data/{connector_cutomer_id}` | `get_revenue_recovery_redis_data` (`revenue_recovery_redis.rs:11-33`) | `handle_get_revenue_recovery_redis_data` (`revenue_recovery_redis.cppm:23-174`) | Up: `Path<String>` id + `Query<GetRedisDataQuery>.key_type` enum (`revenue_recovery_redis.rs:15-20`, `api_models:204-213`); C++: final path segment + strict `key_type` validation, missing id/`key_type` → `INVALID_REQUEST` (`cppm:45-79`) | Up: `ApplicationResponse<Json<RedisDataResponse{exists,ttl_seconds,data}>>` from Redis (`core/revenue_recovery_data_backfill.rs:130-144`); C++: derived `exists`/TTL/typed `data` from cache payload (`cppm:116-173`) | Up: `V2AdminApiAuth` (`revenue_recovery_redis.rs:30`); C++: `AppPermission::ADMIN` (`cppm:29-40`) | Up: real Redis get_ttl + get_key/get_hash_fields, `GenericNotFoundError` on miss (`revenue_recovery_redis_operation.rs:1073-1145`); C++: read-only cache lookup + upstream error message (`cppm:90-114`, `:130-135`) | STUB | Up `routes/revenue_recovery_redis.rs:11-34`, `app.rs:3546`, `:3554`, `:3568-3569`, `core/revenue_recovery_data_backfill.rs:118-155`; C++ `revenue_recovery_redis.cppm:23-174`, route `:176-192`. Path/verb, auth, strict input, derived output and error message match; backing store is a cache stand-in for Redis (external dependency). |

## Not ported
- Real Redis access: upstream obtains a Redis connection and maps failures to `StorageError::RedisError` (`types/storage/revenue_recovery_redis_operation.rs:1078-1084`); C++ never connects to Redis (`revenue_recovery_redis.cppm:72-82`).
- TTL lookup: upstream calls `get_ttl(&redis_key)` and returns the real TTL in `ttl_seconds` (`revenue_recovery_redis_operation.rs:1091-1098`); C++ hardcodes `ttl_seconds=0` (`revenue_recovery_redis.cppm:91`).
- Key-type-specific Redis key construction: upstream `get_connector_customer_lock_key` for `Status` vs `get_connector_customer_tokens_key` for `Tokens` (`revenue_recovery_redis_operation.rs:1086-1089`); C++ only uses key type as a cache-key suffix/label (`revenue_recovery_redis.cppm:49-54`, `:72-73`).
- Status key read (string) and Tokens hash read (`get_hash_fields::<HashMap<String,String>>`) with existence/data derivation (`revenue_recovery_redis_operation.rs:1101-1135`); C++ builds a synthetic JSON envelope from path/query inputs (`revenue_recovery_redis.cppm:56-62`).
- `exists` reflects actual key presence upstream (`revenue_recovery_redis_operation.rs:1101-1135`, `core/revenue_recovery_data_backfill.rs:130-135`); C++ always sets `exists=true` after a self-populated cache hit (`revenue_recovery_redis.cppm:74-90`).
- Error mapping: upstream wraps any Redis error into `ApiErrorResponse::GenericNotFoundError { message: "Redis data not found for connector customer id:- '...'" }` (`core/revenue_recovery_data_backfill.rs:146-154`); C++ emits `AppErrorKind::NOT_FOUND` with a different message and only when the cache entry is absent (`revenue_recovery_redis.cppm:76-81`).
- Strict query deserialization: upstream serde rejects a missing/invalid `key_type` enum (`api_models:204-213`); C++ silently defaults unknown values to `Status` (`revenue_recovery_redis.cppm:50-53`).
- `api_locking::LockAction::NotApplicable` and `api::server_wrap` flow/state plumbing (`revenue_recovery_redis.rs:18-33`); C++ is a direct handler with no wrapping (`revenue_recovery_redis.cppm:23-95`).
- `Flow::RevenueRecoveryRedis` instrumentation (`revenue_recovery_redis.rs:11`, `:18`); C++ records only generic metrics counters (`revenue_recovery_redis.cppm:84-85`).

## Implementation disposition
Re-verified 2026-11-20 against `include/app/handlers/revenue_recovery_redis.cppm`. One line per original `## Not ported` bullet:

- Real Redis access — DEFERRED — no Redis client in C++; read-only `cache().execute` stand-in (`revenue_recovery_redis.cppm:98-114`); real Redis is external dependency.
- TTL lookup — SUBSTITUTED — real remaining TTL from cache `expires_at` instead of Redis `get_ttl` (`revenue_recovery_redis.cppm:138-148`).
- Key-type-specific Redis key construction — IMPLEMENTED — `customer:{id}:status|tokens` built by key type (`revenue_recovery_redis.cppm:81-88`); namespaced cache key at `:88`.
- Status key read / Tokens hash read — SUBSTITUTED — typed JSON decode of cache payload (string vs object/non-empty) instead of Redis `get_key`/`get_hash_fields` (`revenue_recovery_redis.cppm:116-129`).
- `exists` reflects actual key presence — SUBSTITUTED — derived from decoded data (key presence for Status, non-empty object for Tokens) on cache hit (`revenue_recovery_redis.cppm:150-160`).
- Error mapping — IMPLEMENTED — exact upstream `GenericNotFoundError` message and `NOT_FOUND` on cache miss / decode failure (`revenue_recovery_redis.cppm:90-93`, `:108-114`, `:130-135`); trigger is cache miss, not Redis error.
- Strict query deserialization — IMPLEMENTED — missing/invalid `key_type` → `INVALID_REQUEST`, no defaulting to Status (`revenue_recovery_redis.cppm:60-79`); empty id also rejected (`:48-54`).
- `api_locking::LockAction::NotApplicable` / `server_wrap` flow — DEFERRED — direct handler, no wrapping or state plumbing (`revenue_recovery_redis.cppm:23-192`).
- `Flow::RevenueRecoveryRedis` instrumentation — IMPLEMENTED — `metrics().record_flow("RevenueRecoveryRedis")` plus route counter (`revenue_recovery_redis.cppm:162-164`).
