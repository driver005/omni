# Parity: revenue_recovery_data_backfill

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/revenue_recovery_data_backfill.rs` (handlers), `docs/hyperswitch/crates/router/src/routes/app.rs:3542-3578` (mount `/v2/recovery/data-backfill`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/revenue_recovery_data_backfill.rs`; `docs/hyperswitch/crates/router/src/core/revenue_recovery/retry_stats/migration.rs`; `docs/hyperswitch/crates/router/src/core/revenue_recovery/retry_stats/record.rs`
- Upstream storage: `docs/hyperswitch/crates/router/src/types/storage/revenue_recovery_redis_operation.rs`
- Upstream models: `docs/hyperswitch/crates/api_models/src/revenue_recovery_data_backfill.rs`
- C++: `include/app/handlers/revenue_recovery_data_backfill.cppm`
- Checked: 2026-11-20 (re-verified after strict-parity port)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | 4 routes: `POST ""`, `POST /status/:connector_cutomer_id/:payment_intent_id`, `PUT /update-token`, `POST /retry-stats` (cppm:869-905 vs app.rs:3554-3576); no C++-only route exists in this module. |
| Input types | MATCH | Upstream `MultipartForm<RevenueRecoveryDataBackfillForm>` / `RetryStatsMigrationForm` CSV (routes:24,79; api_models:292-380) is parsed in-process from the request body's `file` part or raw CSV (cppm:1637-1672, 1724-1814); `UpdateTokenStatusRequest` + custom `ScheduledAtUpdate` deserializer (api_models:228-262) ported at cppm:664-700; path tuple and `cutoff_time` ISO8601 parse at cppm:441-466 / cppm:36-160. |
| Output types | MATCH | `ApplicationResponse::Json` serializes the inner DTO only (`services/api.rs:489`), so C++ `send_json(encode_json(response))` (cppm:521-524, 631-634, 785-788, 863-866) is wire-identical: `{processed_records,failed_records}`, `{unlocked}`, `{updated,message}`, `{processed_records,failed_records,row_errors}`. DTO fields: `app/dto/revenue_recovery_data_backfill.cppm:71-141,183-210`. |
| Auth/permissions | MATCH | All 4 routes `V2AdminApiAuth` (routes:69,93,124,156) → `authorize()` with `auth_rules().rule_for` + `authorizer().authorize_route` (cppm:949-955). Route-level form/cutoff validation intentionally runs before auth, matching upstream's validate-then-`server_wrap` order (routes:29-71, 83-95). |
| Persistence | MATCH | Upstream `RedisTokenManager` (status SETNX/GET/DEL, tokens hash with TTL) ported verbatim over the injected cache (`LocalRedis`, cppm:254-348); `update_redis_token_with_comprehensive_card_data` (storage:1148-1261) at cppm:999-1064; `get_payment_processor_token_using_token_id` + `update_or_add...` (storage:912-930, 354-403) at cppm:713-752; `replace_retry_stats_document` (retry_stats/record.rs:79-142) at cppm:1375-1396. Real Redis / the retry-stats SQL table are external infra, substituted locally by decision. |
| Domain logic | MATCH | `revenue_recovery_data_backfill` (core:23-64) at cppm:441-529; `process_payment_method_record` (core:263-333) at cppm:967-993; `unlock_connector_customer_status_handler` (core:66-117) at cppm:797-866; `redis_update_additional_details_for_revenue_recovery` (core:157-261) at cppm:641-772; `parse_daily_retry_history` (core:335-388) at cppm:1181-1206; `build_comprehensive_card_data` (core:390-439) at cppm:1075-1119; `determine_card_type` (core:441-464) at cppm:1077-1087; `parse_expiration_date` (core:466-523) at cppm:1122-1160; `migrate_retry_stats_from_csv` + `validate_row` (migration.rs:21-130) at cppm:531-632 / cppm:1216-1255 / cppm:1258-1351. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/v2/recovery/data-backfill` | `revenue_recovery_data_backfill` (routes:19-73) | `handle_revenue_recovery_data_backfill` (cppm:441-529) | Upstream `Query<BackfillQuery>.cutoff_time` + `MultipartForm<RevenueRecoveryDataBackfillForm>` CSV with `failed_records` (routes:29-55; api_models:199-202, 292-332); C++ ISO8601 parse + in-process CSV parser with per-row `CsvParsingError` (cppm:447-489, 1666-1721) | `RevenueRecoveryDataBackfillResponse{processed_records,failed_records}` (core:52-63; cppm:524-529) | V2AdminApiAuth (routes:69) = authorize (cppm:499-502) | Upstream `update_redis_token_with_comprehensive_card_data(..., cutoff_datetime)` (core:306-318; storage:1148-1261); C++ `update_redis_token_with_comprehensive_card_data` with scheduled_at cutoff clearing, modified_at and whole-hash TTL write (cppm:999-1064) | MATCH | routes:19-73 / app.rs:3556-3559; core:23-64, 263-333; storage:1148-1261; cppm:441-529, 967-1064. |
| POST `/v2/recovery/data-backfill/status/{connector_cutomer_id}/{payment_intent_id}` | `revenue_recovery_data_backfill_status` (routes:130-160) | `handle_revenue_recovery_data_backfill_status` (cppm:797-866) | Path `(String, GlobalPaymentId)` → `UnlockStatusRequest` (routes:134-142); C++ last two path segments (cppm:811-823) | `UnlockStatusResponse{unlocked}` (core:108-116; cppm:861-862) | V2AdminApiAuth (routes:156) = authorize (cppm:803-806) | Upstream GET status key → owner check → DEL with typed errors: DeleteFailed→InvalidPaymentIdProvided, other RedisError→InternalServerError (storage:232-286; core:74-106); C++ LocalRedis GET/compare/DEL + same error codes/messages (cppm:833-858, 1844-1858) | MATCH | routes:130-160 / app.rs:3562-3566; core:66-117; storage:232-286; cppm:797-866. |
| PUT `/v2/recovery/data-backfill/update-token` | `update_revenue_recovery_additional_redis_data` (routes:106-128) | `handle_update_revenue_recovery_additional_redis_data` (cppm:641-772) | `UpdateTokenStatusRequest` JSON with string-or-datetime `scheduled_at` (routes:110; api_models:228-262); C++ generic-field parse then typed DTO (cppm:664-700) | `UpdateTokenStatusResponse{updated,message}` (core:246-260; cppm:767-772) | V2AdminApiAuth (routes:124) = authorize (cppm:647-650) | Upstream get token + field updates + hash rewrite with TTL (core:162-238; storage:912-930, 354-403); C++ LocalRedis hash get/update with exact `updated_fields` messages and GenericNotFoundError mapping (cppm:713-752, 754-762) | MATCH | routes:106-128 / app.rs:3571-3573; core:157-261; storage:354-403, 912-930; cppm:641-772. |
| POST `/v2/recovery/data-backfill/retry-stats` | `revenue_recovery_retry_stats_migration` (routes:75-104) | `handle_revenue_recovery_retry_stats_migration` (cppm:531-632) | Upstream `MultipartForm<RetryStatsMigrationForm>` CSV (routes:79; api_models:334-379); C++ in-process CSV parser with the same whole-upload abort / ten-error cap (cppm:1753-1814) | `RetryStatsMigrationResponse{processed_records,failed_records,row_errors}` (migration.rs:69-110; cppm:623-632) | V2AdminApiAuth (routes:93) = authorize (cppm:568-571) | Upstream `validate_row` + duplicate-key rejection + per-key SETNX lock + whole-document replace (migration.rs:21-130; record.rs:79-142); C++ ports all phases with `localRedis` lock and ConfigStore document replace (cppm:579-621, 1216-1396) | MATCH | routes:75-104 / app.rs:3574-3576; migration.rs:21-130; record.rs:79-142; cppm:531-632, 1216-1396. |

## Ported upstream function map
Every upstream function named in the earlier report is now a local helper; none is a stub.

| Upstream function | Upstream evidence | C++ helper |
|---|---|---|
| `revenue_recovery_data_backfill` | core:23-64 | cppm:441-529 |
| `process_payment_method_record` | core:263-333 | cppm:967-993 |
| `unlock_connector_customer_status_handler` | core:66-117 | cppm:797-866 |
| `redis_update_additional_details_for_revenue_recovery` | core:157-261 | cppm:641-772 |
| `parse_daily_retry_history` | core:335-388 | cppm:1181-1206 |
| `build_comprehensive_card_data` | core:390-439 | cppm:1075-1119 |
| `determine_card_type` | core:441-464 | cppm:1077-1087 |
| `parse_expiration_date` | core:466-523 | cppm:1122-1160 |
| `migrate_retry_stats_from_csv` / `validate_row` | migration.rs:21-130 | cppm:531-632 / 1216-1255 |
| `StatsDocument::validate_invariants` | retry_stats_document.rs:68-97 | cppm:1332-1357 |
| `RetryStatsClusterKey::from_db_string` | retry_stats_cluster_key.rs:195-229 | cppm:1216-1250 |
| `replace_retry_stats_document` / `with_retry_stats_lock` / `release_lock` | record.rs:79-108, 226-266, 270-284 | cppm:1375-1396 |
| `RedisTokenManager::unlock_connector_customer_status` | storage:232-286 | cppm:797-866 + LocalRedis cppm:254-348 |
| `RedisTokenManager::update_redis_token_with_comprehensive_card_data` | storage:1148-1261 | cppm:999-1064 |
| `RedisTokenManager::get_payment_processor_token_using_token_id` / `update_or_add_connector_customer_payment_processor_tokens` | storage:912-930 / 354-403 | cppm:713-752 + LocalRedis cppm:304-342 |
| `RevenueRecoveryDataBackfillForm::validate_and_get_records_with_errors` | api_models:298-331 | cppm:1666-1721 |
| `RetryStatsMigrationForm::validate_and_get_records` | api_models:340-379 | cppm:1753-1814 |
| `ScheduledAtUpdate` custom `Deserialize` | api_models:228-253 | cppm:664-700 |

## Local substitute contract (external infra)
Per the strict-parity/local-substitute decision, real Redis, SQL and the multipart framework are replaced by in-process facilities; the observable algorithm and wire shapes are unchanged.

- **Redis keys/TTL**: `customer:{id}:status` and `customer:{id}:tokens` (storage:139-145) namespaced as `revenue_recovery_redis.redis:customer:{id}:...` so the sibling `revenue_recovery_redis.cppm` can read the same raw keys (cppm:269-272). TTL = 3600s; token hash is a single JSON object of token-id → serialized `PaymentProcessorTokenStatus` (storage:21-57), mirroring `HGETALL`/`HSET` string values.
- **Status lock**: `LocalRedis` status key is authoritative; the existing `lock:revenue_recovery.status:{id}` LockRegistry key is also read/released so the sibling handler's documented fallback contract keeps working (cppm:825-832, 858). Upstream's lock is created by the recovery flow, not by this backfill route; the backfill still writes that key as the local stand-in (cppm:985-990).
- **Retry-stats table**: `ConfigStore` namespace `revenue_recovery_retry_stats`, key = cluster key, whole-document replace; lock key `revenue_recovery_retry_stats_lock:{cluster_key}` (record.rs:273). Retry budget 3 attempts / 10 ms / 30 s TTL are local stand-ins for the upstream config values.
- **Time**: `detail::PrimitiveDateTime` (cppm:36-204) provides the two upstream `format_description!` layouts (core:343-358), `Iso8601::DEFAULT` (routes:33-36) and `date_time::now()` (storage:1221). Keys are rendered as `YYYY-MM-DD HH:MM:SS.ffffff`, so comparisons and stored shapes match.
- **Multipart**: `body` is treated as the uploaded `file` (a `name="file"` multipart part is unwrapped at cppm:1609-1623); JSON bodies remain accepted for the pre-parsed DTO path. Row numbering is `row_index + 2` (api_models:320, 358).
- **Errors**: route-level `{"error": ...}` (routes:42-53, 98-102) at cppm:1834-1841; API errors use upstream `ErrorResponse{type,code,message}` (`GenericNotFoundError` IR_37/404, `InvalidPaymentIdProvided` IR_48/404 with the exact double-nested message from core:86 + api_error_response.rs:717, `InternalServerError` HE_00/500) at cppm:1844-1858.
- **Observability**: `#[instrument(Flow::...)]` / `server_wrap` / `api_locking` are represented by `metrics().record_flow` counters and the lock/lock-registry calls (cppm:955-962); this is observability-only and has no wire effect.

## Shared contract requests
None requiring edits to shared files. The following local contracts are relied on and documented so no shared code changes are needed:
1. `revenue_recovery_redis.redis:customer:{id}:status|tokens` cache namespace (shared read-only with `revenue_recovery_redis.cppm`).
2. `lock:revenue_recovery.status:{id}` LockRegistry key (producer here, consumer in `revenue_recovery_redis.cppm`).
3. `ConfigStore` namespace `revenue_recovery_retry_stats` (local retry-stats table stand-in).
Two presentation-only substitutions would need shared support for byte-exact error text: the upstream `hyperswitch_masking::Secret` `Debug` string (use `crypto::SecretString::masked()` here, cppm:711-715/720-735) and `time`'s parse-error `Display` in the invalid-cutoff message (cppm:459-465). Neither affects status codes or JSON field shapes; both are recorded here rather than changing shared `app/crypto` or DTO files.

## Report
1. **Rows moved to MATCH** — all 4 route rows (`POST ""`, `POST /status/{...}/{...}`, `PUT /update-token`, `POST /retry-stats`) and all 6 summary dimensions moved from PARTIAL to MATCH. The previously deferred/substituted upstream functions were actually ported: real datetime parsing and cutoff-based `scheduled_at` clearing, whole-hash token status updates with TTL, secret-aware field updates with upstream error codes, full retry-stats validation (cluster key, invariants, duplicates, per-key lock, whole-document replace), in-process multipart/CSV parsing, and upstream-shaped error payloads. No stubs remain.
2. **Remaining non-MATCH** — none. C++-only routes: none; the module registers exactly the 4 upstream routes (the redis-data GET lives only in the sibling `revenue_recovery_redis.cppm`). Real Redis/SQL/HTTP-multipart instrumentation remain local in-process substitutes by decision, with the observable algorithm and wire shapes ported as documented above.
3. **Shared contract requests** — none blocking; see the three local contracts and two presentation-only substitutions recorded above.
4. **Files changed** — `include/app/handlers/revenue_recovery_data_backfill.cppm` (full strict-parity port with local helpers) and `docs/hyperswitch-parity/revenue_recovery_data_backfill.md` (this report). No DTO/model edit was required; `docs/hyperswitch/` left read-only; other modules untouched; bazel was not run.
