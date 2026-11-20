# Parity: health

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/health.rs`; paths/verbs from `docs/hyperswitch/crates/router/src/routes/app.rs:810-830`
- Upstream core: `docs/hyperswitch/crates/router/src/core/health_check.rs` (no `core/health.rs`; logic lives in `health_check.rs`) + DTO `docs/hyperswitch/crates/api_models/src/health_check.rs`
- Upstream error shape: `docs/hyperswitch/crates/hyperswitch_domain_models/src/errors/api_error_response.rs:60-64,425-427` + `docs/hyperswitch/crates/api_models/src/errors/types.rs:60-70,159-176`
- C++: `include/app/handlers/health.cppm`
- Checked: 2026-11-20 (handler fully ported to strict upstream parity with local substitutes; all 4 previously-PARTIAL rows now MATCH)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | v1 `GET /health` + `/health/ready` (`health.cppm:195-219`); liveness now `Route{""}` so no trailing slash, matching upstream `web::scope("health") + resource("")` (`app.rs:812-819`). v2 `GET /v2/health` + `/v2/health/ready` (`:222-246`, `app.rs:822-829`). Both scopes gated by local constexpr `kV1`/`kV2` (`:29-30`). |
| Input types | MATCH | Both handlers take no payload/params, ignoring the request body (`health.cppm:35-66`). Upstream `()` into `server_wrap` (`health.rs:32-40`). |
| Output types | MATCH | Liveness emits exact plain text `health is good` with `text/plain` (`health.cppm:59`, `:628-640`; `health.rs:16-21`). Deep response JSON is built with exact upstream field order, `HealthState` conversions, feature-gated field presence, and `skip_serializing_if = Option::is_none` on the two Option fields (`health.cppm:170-185`, `:509-558`; `api_models/health_check.rs:3-18,45-61`). |
| Auth/permissions | MATCH | Both operations resolve to `AuthScheme::NO_AUTH` in the registry (`auth/registry.cppm:101,182`); `authorize_route` returns `std::nullopt` for NO_AUTH (`auth/authorizer.cppm:54-62`), so the checks at `health.cppm:44-48,70-74` are exact no-ops — matching upstream liveness (no auth) and `&auth::NoAuth` (`health.rs:16,23-40`). |
| Persistence | MATCH | All nine probes are real local substitutes, no hardcoded NotApplicable: database (`core::db::Database` + `repository().list_payments()`, `health.cppm:282-299`), Redis (`core::cache` put/get/erase, `:301-325`), vault (`keystore().find_record` gated by config, `:327-353`), analytics (`repository().list_refunds()`, `:355-370`), OpenSearch (event bus DB query, `:372-393`), outgoing (`connector_router().select("health_check")->sync`, `:449-476`), decision engine (`decision_engine().evaluate_checked`, `:419-447`), gRPC map (`config_store().list("grpc_health_check")`, `:395-417`), UCS (`superposition().get_config`, `:478-499`). Upstream equivalents: `core/health_check.rs:51-285`. |
| Domain logic | MATCH | `HealthStateProjection::as_bool` / `as_optional` implement exact `From<HealthState>` conversions (Running→true/Some(true), Error→false/Some(false), NotApplicable→false/None) (`health.cppm:260-278`; `api_models/health_check.rs:45-61`). Probe order and early-return-on-error mirror `deep_health_check_func` (`health.rs:51-166`). Per-component errors use the exact upstream wire shape and HTTP 500 (`health.cppm:568-584`). Flow+counter metrics on both endpoints (`:55-56,81-82`). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/health` (v1 liveness) | `health` | `handle_health` (`health.cppm:35-60`, mounted `:198`) | none | Up: plain text `health is good`; C++: plain text `health is good`, `text/plain` (`:59`, `:628-640`) | Up: none; C++: NO_AUTH rule, no-op (`:44-48`) | none | MATCH | Up `routes/health.rs:14-21`, `app.rs:812-819`; C++ `health.cppm:35-60`, `:195-219` |
| GET `/health/ready` (v1 deep) | `deep_health_check` → `deep_health_check_func` | `handle_deep_health_check` (`health.cppm:62-186`, mounted `:209`) | none | Up: `RouterHealthCheckResponse` (`ApplicationResponse::Json`); C++: byte-equivalent JSON with exact field order/gating/Option omission (`:170-185`, `:509-558`) | Up: `NoAuth` + `LockAction::NotApplicable` (`health.rs:32-40`); C++: NO_AUTH rule, no-op (`:70-74`) | Up: DB + Redis + locker + (olap) analytics/opensearch + outgoing + (dynamic_routing) gRPC/decision + UCS; C++: nine real local probes (`:282-499`) | MATCH | Up `routes/health.rs:23-185`, `app.rs:818`, `core/health_check.rs:50-286`; C++ `health.cppm:62-186`, `:260-558` |
| GET `/v2/health` (v2 liveness) | `health` | `handle_health` (`health.cppm:35-60`, mounted `:225`) | none | Up: plain text; C++: plain text (`:59`) | Up: none; C++: NO_AUTH rule, no-op (`:44-48`) | none | MATCH | Up `app.rs:822-827`; C++ `health.cppm:222-234` |
| GET `/v2/health/ready` (v2 deep) | `deep_health_check` | `handle_deep_health_check` (`health.cppm:62-186`, mounted `:236`) | none | Up: `RouterHealthCheckResponse`; C++: same deep JSON (`:170-185`) | Up: `NoAuth`; C++: NO_AUTH rule, no-op (`:70-74`) | same as v1 deep | MATCH | Up `app.rs:828`; C++ `health.cppm:235-246` |

## Not ported (all resolved this pass)
- ~~Output body shape for liveness~~ — RESOLVED: plain text (see Implementation disposition).
- ~~Authorization semantics~~ — RESOLVED: both registry operations are `NO_AUTH` (`registry.cppm:101,182`), `authorize_route` no-ops (`authorizer.cppm:60-62`).
- ~~Metrics `HEALTH_METRIC.add(1, &[])`~~ — RESOLVED: `record_flow` + `counter` on both endpoints (`health.cppm:55-56,81-82`).
- ~~`HealthCheckInterface` / `HealthState` mapping~~ — RESOLVED: `HealthStateProjection` (`health.cppm:260-278`).
- ~~Real DB probe~~ — RESOLVED: `probe_database()` (`health.cppm:282-299`).
- ~~Locker/vault probe~~ — RESOLVED: `probe_vault()` probes the keystore when active (`health.cppm:327-353`).
- ~~Outgoing HTTP probe~~ — RESOLVED: `probe_outgoing()` (`health.cppm:449-476`).
- ~~Analytics probe~~ — RESOLVED: `probe_analytics()` (`health.cppm:355-370`).
- ~~OpenSearch probe~~ — RESOLVED: `probe_opensearch()` (`health.cppm:372-393`).
- ~~gRPC `HealthCheckMap`~~ — RESOLVED: `probe_grpc_map()` populates the map (`health.cppm:395-417`), emitted at `:537-549`.
- ~~Decision Engine probe~~ — RESOLVED: `probe_decision_engine()` (`health.cppm:419-447`).
- ~~Unified Connector Service probe~~ — RESOLVED: `probe_unified_connector_service()` with SERVING/NOT_SERVING and configured-client fallback (`health.cppm:478-499`).
- ~~Per-component error context mapping~~ — RESOLVED: exact `{"type":"api","message":"{component} health check failed with error: {message}","code":"HE_00"}` + HTTP 500 (`health.cppm:568-584`), invoked for all nine components (`:93-168`).
- ~~v2 route surface~~ — RESOLVED (`:222-246`).
- ~~Feature-gated DTO field presence~~ — RESOLVED locally: `encode_health_response` gates analytics/opensearch on `kOlap` and grpc_health_check/decision_engine on `kDynamicRouting`, omits None Option fields (`health.cppm:27-30`, `:509-558`).

## Implementation disposition
- Liveness body — IMPLEMENTED — `send_text(res, send, "health is good")` with `text/plain` (`health.cppm:59`, `:628-640`); no longer JSON. Upstream `health.rs:16-21`.
- Authorization semantics — IMPLEMENTED — both ops resolve to `AuthScheme::NO_AUTH` (`registry.cppm:101,182`); `authorize_route` returns `std::nullopt` for NO_AUTH (`authorizer.cppm:60-62`), so `health.cppm:44-48,70-74` are exact no-ops matching upstream no-auth / `&auth::NoAuth`.
- Metrics — IMPLEMENTED — `metrics().record_flow(...)` + `counter("health").increment()` on both endpoints (`health.cppm:55-56,81-82`); local substitute for `HEALTH_METRIC.add(1, &[])`.
- `HealthState` mapping — IMPLEMENTED — `HealthStateProjection::as_bool` / `as_optional` (`health.cppm:260-278`), exact conversions from `api_models/health_check.rs:45-61`.
- Database probe — IMPLEMENTED — `probe_database()` uses `database().is_open()` + `repository().list_payments()` (`health.cppm:282-299`), failure → `HealthCheckError{component:"Database"}`.
- Locker/vault probe — IMPLEMENTED — `probe_vault()` reads `health_check/locker_enabled` and `health_check/mock_locker` config, probes `key_store().find_record(...)` when active, else `NotApplicable` (`health.cppm:327-353`); mirrors `core/health_check.rs:87-105`.
- Analytics probe (olap) — IMPLEMENTED — `probe_analytics()` runs a real DB-backed repository read (`health.cppm:355-370`); mirrors `core/health_check.rs:107-144`.
- OpenSearch probe (olap) — IMPLEMENTED — `probe_opensearch()` queries the DB-backed event bus, with a config-based disabled-client fallback to `NotApplicable` (`health.cppm:372-393`); mirrors `core/health_check.rs:146-159`.
- gRPC `HealthCheckMap` (dynamic_routing) — IMPLEMENTED — `probe_grpc_map()` builds the service→bool map from `config_store().list("grpc_health_check")` (`health.cppm:395-417`); mirrors `core/health_check.rs:181-195`.
- Decision Engine probe (dynamic_routing) — IMPLEMENTED — `probe_decision_engine()` evaluates a trivial program via the real local `DecisionEngine` (`health.cppm:419-447`); mirrors `core/health_check.rs:197-221`.
- Unified Connector Service probe — IMPLEMENTED — `probe_unified_connector_service()` consults `superposition().get_config("grpc_clients","unified_connector_service")` with SERVING→Running, NOT_SERVING→Error, absent→NotApplicable (`health.cppm:478-499`); mirrors `core/health_check.rs:223-285`.
- Outgoing probe — IMPLEMENTED — `probe_outgoing()` selects and calls a configured connector synchronously (`health.cppm:449-476`); mirrors `core/health_check.rs:161-179`.
- Per-component error context — IMPLEMENTED — `send_health_error()` emits the exact upstream error body (`{"type":"api","message":"{component} health check failed with error: {message}","code":"HE_00"}`) with HTTP 500 (`health.cppm:568-584`, error derivation `api_error_response.rs:60-64,425-427`, serialization `errors/types.rs:60-70,159-176`); invoked for all components at `health.cppm:93-168`.
- v2 route surface — IMPLEMENTED — `Router{router, "/v2/health"}` registers both liveness and deep (`health.cppm:222-246`).
- Feature-gated field presence — IMPLEMENTED locally — `encode_health_response` gates `analytics`/`opensearch` on `kOlap` and `grpc_health_check`/`decision_engine` on `kDynamicRouting`, and omits `vault`/`unified_connector_service` when `None` (`health.cppm:27-30`, `:509-558`); exact field order per `api_models/health_check.rs:3-18`.
- v1 liveness trailing slash — IMPLEMENTED — changed v1 liveness from `Route{"/"}` to `Route{""}` so the path is exactly `/health` (`health.cppm:198`); upstream `app.rs:815-817`.

## Shared contract requests
- `app/dto/health_check.cppm` (outside this module, unedited): `RouterHealthCheckResponse` currently has no `skip_serializing_if` on `vault`/`unified_connector_service` and no cfg-equivalent gating for the olap/dynamic_routing fields. The handler works around this by emitting the response JSON locally in `encode_health_response` (`health.cppm:509-558`). If the DTO gains `FieldOptions` for optional-omission and gated fields, the handler can switch back to `serde::Ser::encode_json(response)` with no wire change. No shared change is required for this module to be MATCH.
- `app/auth/registry.cppm` already registers `health` and `deep_health_check` as `NO_AUTH` (`registry.cppm:101,182`) — no change needed. Confirm these two entries stay `NO_AUTH` so the handler's no-op checks remain exact.

## Files changed
- `include/app/handlers/health.cppm` — full port of `routes/health.rs` + `core/health_check.rs` as local helpers; real probes, exact `HealthState` conversions, exact error shape, local constexpr feature gates, exact plain-text liveness, v1 path fix, v2 routes.
- `docs/hyperswitch-parity/health.md` — this report (all 4 PARTIAL rows moved to MATCH; summary tables updated; disposition and shared requests recorded).
