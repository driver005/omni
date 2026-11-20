# Parity: feature_matrix

- Upstream route: `docs/hyperswitch/crates/router/src/routes/feature_matrix.rs:1-161` (all logic;
  there is no `core/feature_matrix.rs`). Path/verb + gate from
  `docs/hyperswitch/crates/router/src/routes/app.rs:3428-3437` (`web::scope("/feature_matrix")` +
  `web::resource("")` + `web::get()` -> `fetch_feature_matrix`, gated
  `all(feature = "olap", feature = "v1")` at `app.rs:3430`).
- Upstream wire types: `docs/hyperswitch/crates/api_models/src/feature_matrix.rs:6-95`;
  connector conversion `docs/hyperswitch/crates/router/src/types/api/feature_matrix.rs:16-522`.
- C++: `include/app/handlers/feature_matrix.cppm`; DTO `include/app/dto/feature_matrix.cppm:18-129`.
- Checked: 2026-11-20. Strict-parity mode: every route row driven to upstream-identical MATCH.

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | One route `GET /feature_matrix/` (`feature_matrix.cppm:2162-2188`, route `:2178`) matches upstream `GET /feature_matrix` (`app.rs:3433-3435`). C++-only `/generate_feature_matrix` removed from `install()`. `olap`+`v1` gate implemented with a local constexpr (`feature_matrix.cppm:2088`) checked in `install()` (`:2164-2172`), equivalent to `app.rs:3430`. |
| Input types | MATCH | Both take `{connectors?: [...]}` with `None` = all connectors. Upstream `Option<Json<FeatureMatrixRequest>>` / `Option<Vec<Connector>>` (`routes/feature_matrix.rs:23-28`, `api_models/feature_matrix.rs:6-11`); C++ validates snake_case enum strings against the exact `Connector::iter()` wire-name set (`feature_matrix.cppm:2203-2235`, registry `:1909-2080`) into a plain-string DTO (`dto/feature_matrix.cppm:36-43`), matching serde's `Vec<Connector>` scalar shape. |
| Output types | MATCH | `FeatureMatrixListResponse{connector_count, connectors}` and all nested fields ported with plain string/array wire values (`dto/feature_matrix.cppm:18-129`, serde `:132-210`); every field populated in `feature_matrix.cppm:2318-2331,2375-2404`. Upstream `api_models/feature_matrix.rs:33-92`; `#[serde(flatten)]` card features reproduced as top-level optional fields (`dto/feature_matrix.cppm:63-86`). |
| Auth/permissions | MATCH | `&auth::NoAuth` + `LockAction::NotApplicable` (`routes/feature_matrix.rs:30-38`) -> NO_AUTH `rule_for`/`authorize_route` pass-through with no lock (`feature_matrix.cppm:2152-2159`); `Flow::FeatureMatrix` instrumentation (`#[instrument]`, `:19,25`) -> `metrics().record_flow` + counter (`:2156-2157`). |
| Persistence | MATCH | Upstream reads no DB; it uses the compile-time connector registry + `state.conf` (`routes/feature_matrix.rs:46-48,80-161`). C++ uses the transcribed 151-entry capability catalog (`feature_matrix.cppm:38-1653`), the 249-entry alpha2->alpha3 map (`:1656-1906`) and the 170-name `Connector::iter()` registry (`:1909-2080`), with config_store lookups for `base_url`/`pm_filters` (`:2324-2326,2351-2373`). No merchant-connector DB read on this route. |
| Domain logic | MATCH | `generate_feature_matrix`, `build_connector_feature_details`, `build_payment_method_wise_feature_details`, `convert_connector` ported verbatim (`feature_matrix.cppm:2196-2260,2281-2332,2334-2405,2262-2279`) against upstream `routes/feature_matrix.rs:42-161` and `types/api/feature_matrix.rs:16-522`; includes `warn!` on conversion failure and `Connector::about == None` filtering. |

## Routes
Upstream registers exactly one route: `GET /feature_matrix` -> `fetch_feature_matrix`
(`app.rs:3433-3435`). `generate_feature_matrix` is only an internal helper called by
`fetch_feature_matrix` (`routes/feature_matrix.rs:35,42`), never an HTTP endpoint. C++ now
registers exactly that one route; the former C++-only `GET /generate_feature_matrix` alias was
removed from `install()` and from this report.

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/feature_matrix` (C++ `/feature_matrix/`) | `fetch_feature_matrix` -> `generate_feature_matrix` | `handle_fetch_feature_matrix` -> private `generate_feature_matrix` | Up: `Option<Json<FeatureMatrixRequest>>`, default `{connectors:None}` (`routes/feature_matrix.rs:23-28`); C++: snake_case connector strings validated against the exact enum wire-name registry (`:2203-2235`, `:1909-2080`) | Up: `ApplicationResponse::Json(FeatureMatrixListResponse)` (`:67-72`); C++: `FeatureMatrixListResponse` JSON, 200, all fields from upstream static tables (`:2240-2259`, `:2318-2331`, `:2375-2404`) | Up: `NoAuth`, `LockAction::NotApplicable` (`:30-38`), `Flow::FeatureMatrix` (`:19,25`); C++: NO_AUTH pass-through, no lock, `record_flow`/counter (`:2152-2159`) | Up: static connector registry + `state.conf` (`:46-48,80-161`); C++: transcribed 151-entry catalog + alpha map + registry, config_store for `base_url`/`pm_filters` (`:38-2080`, `:2324-2373`) | MATCH | Up `routes/feature_matrix.rs:20-161`, `app.rs:3430-3435`; C++ `feature_matrix.cppm:2143-2188,2196-2405`, DTO `dto/feature_matrix.cppm:18-210` |

## Ported upstream functions (previously under "Not ported")
All items listed in the original report's "Not ported" section are now implemented:

- `generate_feature_matrix` (`routes/feature_matrix.rs:42-73`) — ported as the private
  `generate_feature_matrix` (`feature_matrix.cppm:2196-2260`): `unwrap_or_else(Connector::iter())`,
  `filter_map` over `convert_connector(...).inspect_err(warn).ok().and_then(build_...)`, and
  `ApplicationResponse::Json(FeatureMatrixListResponse)` with `connector_count`.
- `build_connector_feature_details` (`routes/feature_matrix.rs:75-112`) — ported at
  `feature_matrix.cppm:2281-2332`: maps `get_supported_payment_methods` through
  `build_payment_method_wise_feature_details`, reads `get_supported_webhook_flows`, and applies
  `connector_about.map(...)` (returns `std::nullopt` when `get_connector_about()` is `None`,
  dropping ctp_mastercard/vgs/recurly/etc. exactly as upstream). `name = to_uppercase()` (`:102`),
  `display_name`/`description` from `ConnectorInfo`, `category`/`integration_status` from the same,
  and `base_url = Some(connector.base_url(&conf.connectors))` (`:102-109`).
- `build_payment_method_wise_feature_details` (`routes/feature_matrix.rs:114-161`) — ported at
  `feature_matrix.cppm:2334-2405`: per payment-method-type row from the transcribed
  `SupportedPaymentMethods` tables, with `state.conf.pm_filters` country/currency lookups,
  `CountryAlpha2::from_alpha2_to_alpha3` conversion (249-entry map at `:1656-1906`), mandates,
  refunds, capture methods and `specific_features` (flattened card fields).
- `Connector::iter()` registry — the exact 170 strum snake_case wire names from
  `common_enums/src/connector_enums.rs:32` transcribed at `feature_matrix.cppm:1909-2080`.
- `get_connector_about()` / `get_supported_payment_methods()` /
  `get_supported_webhook_flows()` / `base_url()` — their static per-connector tables are
  transcribed from `hyperswitch_connectors/src/connectors/*.rs` into the 151-entry catalog at
  `feature_matrix.cppm:38-1653`; `base_url` comes from the config_store equivalent of
  `state.conf.connectors` (`:2324-2326`).
- `FeatureMatrixConnectorData::convert_connector` + `warn!` — validation at
  `feature_matrix.cppm:2262-2279`, rejecting unknown names and the explicit
  `Cardinal | Juspay | Nsure` error arm (`types/api/feature_matrix.rs:512-516`) with a `warn`
  log (`routes/feature_matrix.rs:57-59`).
- `server_wrap` middleware — `#[instrument]`/`Flow::FeatureMatrix`, `NoAuth`,
  `LockAction::NotApplicable` reproduced at `feature_matrix.cppm:2149-2159` (no lock on a
  read-only GET).
- Feature gate — local constexpr `kOlapV1Enabled` (`:2088`) plus the existing config gate facility
  in `install()` (`:2164-2172`), equivalent to `all(feature = "olap", feature = "v1")`
  (`app.rs:3430`).
- C++-only `generate_feature_matrix` route — removed; `install()` registers only the single
  upstream route (`:2162-2188`).

## Implementation disposition

- `generate_feature_matrix` — IMPLEMENTED — `feature_matrix.cppm:2196-2260`.
- `build_connector_feature_details` — IMPLEMENTED — `feature_matrix.cppm:2281-2332`.
- `build_payment_method_wise_feature_details` — IMPLEMENTED — `feature_matrix.cppm:2334-2405`.
- Default connector registry (`Connector::iter()`) — IMPLEMENTED — 170 names at `:1909-2080`.
- Static capability tables (`get_connector_about` / `get_supported_payment_methods` /
  `get_supported_webhook_flows` / `base_url`) — IMPLEMENTED — 151-entry catalog at `:38-1653`.
- `convert_connector` validation + `warn!` — IMPLEMENTED — `:2262-2279`.
- `server_wrap` middleware (Flow/NoAuth/NoLock) — IMPLEMENTED — `:2149-2159`.
- Feature gating (`olap`+`v1`) — IMPLEMENTED — constexpr `:2088`, `install()` `:2164-2172`.
- C++ extra `generate_feature_matrix` route — REMOVED — `install()` `:2162-2188`.
- Wire DTOs — IMPLEMENTED — plain string/array members so rfl emits upstream-identical JSON
  scalars (`dto/feature_matrix.cppm:18-129`, serde `:132-210`).

## Shared contract requests

No shared code changes. All changes are confined to the exclusive handler and DTO files; no
shared module (`all.cppm`, `routes.cppm`, `base.cppm`, `context/`, `services/`, `auth/`,
`connector/`) was modified. The compile-time `olap`+`v1` gate uses a local constexpr plus the
existing config-store gate facility, so no shared build-flag plumbing is required. The `base_url`
config namespace (`connector_base_url`) and `pm_filters` key layout are local conventions of this
port, wired entirely inside the handler.

Shared-doc bookkeeping only (not edited here because `README.md` is outside this module's file
scope): the aggregate counts in `docs/hyperswitch-parity/README.md:14-17` should move this module's
old `PARTIAL`+`MISSING` rows to `MATCH` (MATCH 114 -> 116, PARTIAL 700 -> 698, MISSING 9 -> 8)
when the owning session consolidates reports.

## Files changed

- `include/app/handlers/feature_matrix.cppm` — faithful port of the upstream route plus the
  generated static capability catalog, alpha map and connector registry; single gated route.
- `include/app/dto/feature_matrix.cppm` — wire DTOs switched to plain string/array members to
  emit upstream-identical JSON (including flattened card-specific fields).
- `docs/hyperswitch-parity/feature_matrix.md` — this report.
