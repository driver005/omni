# Parity: health

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/health.rs`; paths/verbs from `docs/hyperswitch/crates/router/src/routes/app.rs:810-830`
- Upstream core: `docs/hyperswitch/crates/router/src/core/health_check.rs` (no `core/health.rs`; logic lives in `health_check.rs`) + DTO `docs/hyperswitch/crates/api_models/src/health_check.rs`
- C++: `include/app/handlers/health.cppm`
- Checked: 2026-11-20 (handler re-verified against current source)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | PARTIAL | v1 liveness+ready present (`/health/`, `/health/ready`); v2 `/v2/health` + `/v2/health/ready` now registered (`health.cppm:149-172`). v1 liveness still trailing slash vs upstream `/health`. |
| Input types | MATCH | Both handlers take no payload/params. Upstream `()` into `server_wrap` (`health.rs:36`); C++ ignores request body (`health.cppm:23-27`, `:47-51`). |
| Output types | PARTIAL | Liveness now emits plain text `health is good` (`health.cppm:44`, `:321-333`), matching upstream. Deep DTO field set aligns, but `grpc_health_check` never set and several fields report hardcoded NotApplicable (`health.cppm:88-116`). Documented substitutes present → PARTIAL. |
| Auth/permissions | PARTIAL | Upstream liveness has no auth and deep uses `&auth::NoAuth` (`health.rs:16`, `:38`). C++ gates both via `auth_rules().rule_for(...)` + `authorize_route(...)` (`health.cppm:29-33`, `:53-57`) — stricter than upstream. |
| Persistence | PARTIAL | DB probe is documented local substitute `database().is_open()` + `repository().list_payments()` with `HealthCheckError` mapping (`health.cppm:206-223`); Redis real put/get/erase (`:225-249`); outgoing via `connector_router().select("health_check")`/`sync` (`:251-278`); locker/analytics/opensearch/decision/UCS report NotApplicable. |
| Domain logic | PARTIAL | `HealthStateProjection` maps Running→true, Error→false, NotApplicable→false/None (`health.cppm:186-204`); per-component `HealthCheckError` mapping (`:280-292`); flow+counter metrics (`:39-40`, `:63-64`). olap/dynamic_routing components still not probed → PARTIAL, no longer STUB. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/health` (v1 liveness) | `health` | `handle_health` (mounted at `/health/`) | none | Up: plain text `health is good`; C++: plain text `health is good`, `text/plain` (`health.cppm:44`, `:321-333`) — output MATCH | Up: none; C++: rule-based auth (`health.cppm:29-33`) | none | PARTIAL (auth only) | Up `routes/health.rs:16-21`, `app.rs:817`; C++ `health.cppm:23-45`, `:125-136` |
| GET `/health/ready` (v1 deep) | `deep_health_check` → `deep_health_check_func` | `handle_deep_health_check` | none | Up: `RouterHealthCheckResponse` (`ApplicationResponse::Json`); C++: `dto::health_check::RouterHealthCheckResponse` with HealthState projection (`health.cppm:66-118`), grpc unset (`:116`) | Up: `NoAuth` via `server_wrap`, `LockAction::NotApplicable`; C++: rule-based auth (`health.cppm:53-57`) | Up: store DB + Redis + locker HTTP + (olap) analytics/opensearch + outgoing HTTP + (dynamic_routing) gRPC/decision engine + UCS; C++: DB substitute + `list_payments()`, cache put/get/erase, connector `sync`; others NotApplicable (`health.cppm:206-278`) | PARTIAL | Up `routes/health.rs:23-42`, `:44-185`, `app.rs:818`, `core/health_check.rs:50-286`; C++ `health.cppm:47-119`, `:186-292`, `:137-147` |
| GET `/v2/health` (v2 liveness) | `health` | `handle_health` | none | Up: plain text; C++: plain text (`health.cppm:44`) | Up: none; C++: rule-based auth (`health.cppm:29-33`) | none | PARTIAL (auth) | Up `app.rs:825-827`; C++ `health.cppm:150-161` |
| GET `/v2/health/ready` (v2 deep) | `deep_health_check` | `handle_deep_health_check` | none | Up: `RouterHealthCheckResponse`; C++: same DTO as v1 deep (`health.cppm:107-118`) | Up: `NoAuth`; C++: rule-based auth (`health.cppm:53-57`) | same as v1 deep | PARTIAL | Up `app.rs:828`; C++ `health.cppm:162-172` |

## Not ported
- Output body shape for liveness: upstream returns `HttpResponse::Ok().body("health is good")` (plain text), C++ returns serialized JSON `{"status":"health is good"}` (`routes/health.rs:16-21`; `health.cppm:36-39`).
- Authorization semantics: upstream liveness has no auth and deep uses `&auth::NoAuth` (`routes/health.rs:16`, `:38`); C++ gates both behind `AppPermission::ADMIN` token check (`health.cppm:27-34`, `:48-55`).
- Metrics `HEALTH_METRIC.add(1, &[])` incremented on both endpoints (`routes/health.rs:17`, `:28`). Absent in C++.
- `HealthCheckInterface` trait and `HealthState` → `bool`/`Option<bool>` mapping (`Running`→true/Some(true), `Error`→false/Some(false), `NotApplicable`→false/None): `core/health_check.rs:16-47`, `api_models/health_check.rs:39-61`. C++ uses raw bools/nullopt with no state model (`health.cppm:61-93`).
- Real DB health probe `store.health_check_db()` (`core/health_check.rs:51-55`); C++ substitutes `repository().list_payments()` (`health.cppm:62-67`).
- Locker/vault probe — enabled-locker HTTP call to `LOCKER_HEALTH_CALL_PATH`, else `NotApplicable` (`core/health_check.rs:87-105`). C++ hardcodes `vault = nullopt` (`health.cppm:88`).
- Outgoing HTTP probe to `OUTGOING_CALL_URL` via `call_connector_api` with error message mapping (`core/health_check.rs:161-179`). C++ hardcodes `outgoing_request = false` (`health.cppm:91`).
- Analytics probe across `Sqlx`/`Clickhouse`/`CombinedCkh`/`CombinedSqlx` providers (`core/health_check.rs:107-144`, olap-gated). C++ hardcodes `analytics = false` (`health.cppm:89`).
- OpenSearch probe via `opensearch_client.deep_health_check()` (`core/health_check.rs:146-159`, olap-gated). C++ hardcodes `opensearch = false` (`health.cppm:90`).
- gRPC health check `HealthCheckMap` via `health_client.perform_health_check(...)` (`core/health_check.rs:181-195`, dynamic_routing-gated). C++ DTO has field but handler never populates it (`health.cppm:39-40`, handler `:85-93`).
- Decision Engine HTTP probe with `NotApplicable` fallback (`core/health_check.rs:197-221`, dynamic_routing-gated). C++ hardcodes `decision_engine = false` (`health.cppm:92`).
- Unified Connector Service gRPC health check (SERVING/NOT_SERVING, configured-client fallback): `core/health_check.rs:223-285`. C++ hardcodes `unified_connector_service = nullopt` (`health.cppm:93`).
- Per-component error context mapping to `ApiErrorResponse::HealthCheckError { component, message }` for Database/Redis/Locker/Analytics/gRPC/Decision Engine/Opensearch/Outgoing/UCS (`routes/health.rs:51-166`). C++ swallows all exceptions into `false` (`health.cppm:61-83`).
- v2 route surface `/v2/health` and `/v2/health/ready` (`app.rs:822-829`). No C++ registration under that scope (`health.cppm:99-125`).
- Feature-gated DTO field presence: upstream `RouterHealthCheckResponse` conditionally includes `analytics`/`opensearch` (olap) and `grpc_health_check`/`decision_engine` (dynamic_routing) (`api_models/health_check.rs:8-17`). C++ DTO always serializes all fields (`app/dto/health_check.cppm:45-53`, `:77-85`).

## Implementation disposition
- Output body shape for liveness — IMPLEMENTED — `send_text(res, send, "health is good")` with `text/plain` (`health.cppm:44`, `:321-333`); no longer JSON.
- Authorization semantics — DEFERRED — still rule-based `auth_rules().rule_for(...)`/`authorize_route(...)` on both handlers (`health.cppm:29-33`, `:53-57`); upstream no-auth/NoAuth unchanged.
- Metrics `HEALTH_METRIC.add(1, &[])` — IMPLEMENTED — `metrics().record_flow(...)` + `counter("health").increment()` on both endpoints (`health.cppm:39-40`, `:63-64`); local metrics API substitute.
- `HealthCheckInterface` / `HealthState` mapping — IMPLEMENTED — `HealthStateProjection::as_bool`/`as_optional` implements Running→true, Error→false, NotApplicable→false/None (`health.cppm:186-204`); states used at `:66-115`.
- Real DB health probe `store.health_check_db()` — SUBSTITUTED — `probe_database()` uses `database().is_open()` + `repository().list_payments()` with failure mapped to HealthCheckError (`health.cppm:206-223`, comment `:69-71`); no upstream-equivalent DB health-check API available locally.
- Locker/vault probe — SUBSTITUTED — no locker/vault client configured (external dependency), so `vault_state = NotApplicable` → `nullopt` (`health.cppm:88`, `:110`), matching upstream disabled/mocked fallback.
- Outgoing HTTP probe `call_connector_api(OUTGOING_CALL_URL)` — SUBSTITUTED — `probe_outgoing()` routes a sync probe through `connector_router().select("health_check")` → `connector->sync(...)` (`health.cppm:251-278`, result `:113`); no direct URL-based connector API locally.
- Analytics probe (olap) — SUBSTITUTED — no analytics/olap client wired (external dependency): `analytics_state = NotApplicable` → `false` (`health.cppm:98`, `:111`), matching upstream absent-client conversion.
- OpenSearch probe (olap) — SUBSTITUTED — no opensearch client wired (external dependency): `opensearch_state = NotApplicable` → `false` (`health.cppm:99`, `:112`).
- gRPC health check `HealthCheckMap` — DEFERRED — no gRPC health client wired (external dependency); map intentionally left empty, field never populated (`health.cppm:101-105`, `:116`).
- Decision Engine HTTP probe — SUBSTITUTED — no dynamic_routing client configured (external dependency): `decision_state = NotApplicable` → `false` (`health.cppm:104`, `:114`), matching upstream fallback for unconfigured feature.
- Unified Connector Service gRPC check — SUBSTITUTED — no gRPC client configured (external dependency): `ucs_state = NotApplicable` → `nullopt` (`health.cppm:105`, `:115`), matching configured-client fallback.
- Per-component error context mapping — IMPLEMENTED — `send_health_error()` builds `HealthCheckError for {component}: {message}` (`health.cppm:280-292`), invoked for Database (`:73-76`) and Redis (`:80-83`); applies only to probing components.
- v2 route surface — IMPLEMENTED — `Router{router, "/v2/health"}` registers both liveness (`Route{""}`) and deep (`Route{"/ready"}`) (`health.cppm:149-172`).
- Feature-gated DTO field presence — DEFERRED — handler sets all DTO fields unconditionally (`health.cppm:107-115`); conditional serialization lives in `app/dto/health_check.cppm` (outside this handler file), unchanged.
