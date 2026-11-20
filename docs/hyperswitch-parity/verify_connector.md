# Parity: verify_connector

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/verify_connector.rs` (`payment_connector_verify`); registered in `docs/hyperswitch/crates/router/src/routes/app.rs:2244-2245` under `web::scope("/account")`
- Upstream core: `docs/hyperswitch/crates/router/src/core/verify_connector.rs` (`verify_connector_credentials`); helpers in `core/verify_connector.rs:19-67`, `utils/verify_connector.rs:6-55`, `types/api/verify_connector.rs:16-246`, `api_models/src/verify_connector.rs:5-9`, `common_enums/src/connector_enums.rs:29-235`
- C++: `include/app/handlers/verify_connector.cppm` (handler + local upstream helpers), `include/app/dto/verify_connector.cppm` (typed request DTO)
- Checked: 2026-11-19
- Re-verified: 2026-11-20 (strict-parity port; every named upstream function ported as a local helper with file:line evidence; all rows MATCH)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `POST /account/connectors/verify` both sides (app.rs:2244-2245; cppm:480-496). |
| Input types | MATCH | Typed `VerifyConnectorRequest`: `connector_name` validated into the upstream `Connector` enum (snake_case wire table) and structured `ConnectorAuthType` object; decoded with `deserialize` (dto.cppm:27-198,363-389,390-443,465-477; cppm:395-404). |
| Output types | MATCH | Upstream `RouterResponse<()>` / `StatusOk` with no body; C++ empty `Status::OK` (cppm:474,517-529). |
| Auth/permissions | MATCH | JWT `MerchantConnectorWrite`/`CONNECTOR_WRITE`, `allow_connected=true`, `allow_platform=false`; `auth.profile.id` read as `profile_id` (routes:31-35,28; cppm:371-387). |
| Persistence | MATCH | Read-only connector resolution from config; no DB write, no merchant-account lookup, no cache write — upstream performs none (core:24-30; cppm:181-195,420-424). |
| Domain logic | MATCH | Every named upstream function ported as a local helper: `get_connector_by_name`, `get_test_card_details`, `generate_card_from_details`, `foreign_into`, `VerifyConnectorData`, `get_payment_authorize_data`, `get_router_data`, `Stripe::verify`, `Paypal::get_access_token`, `handle_payment_error_response` (core:19-67; utils:6-55; types:16-246; cppm:34-354). External PSP calls use the documented local in-process connector substitute. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/account/connectors/verify` | `payment_connector_verify` (routes:13-39) → `verify_connector_credentials` (core:19-67) | `handle_payment_connector_verify` (cppm:365-475), installed cppm:480-496 | `web::Json<VerifyConnectorRequest>` (routes:16); typed `connector_name: Connector`, `connector_account_details: ConnectorAuthType` (api_models:5-9) | `RouterResponse<()>` → `StatusOk` (core:23,60); C++ empty 200 (cppm:474) | `JWTAuth{MerchantConnectorWrite, allow_connected:true, allow_platform:false}` (routes:31-35); `api_locking::NotApplicable` (routes:36) | `ConnectorData::get_connector_by_name(state.conf.connectors,...)` (core:24-30) → config resolve; `Stripe::verify` / `Paypal::get_access_token` (core:41,51) → local in-process connector; Flow `VerifyPaymentConnector` (routes:12,18) | MATCH | Upstream: app.rs:2244-2245, routes:13-39, core:19-67, utils:6-55, types/api/verify_connector.rs:16-246, api_models:5-9; C++: cppm:34-354,365-475,480-496; dto.cppm:27-198,363-477. |

### Dimension evidence detail
- **Route paths/verbs (MATCH):** Upstream scope `/account` + resource `/connectors/verify` + `web::post()` (app.rs:2236,2244-2245); C++ `Router{router,"/account"}.add_route(..."/connectors/verify").post(...)` (cppm:480-496).
- **Input types (MATCH):** Upstream decodes `VerifyConnectorRequest { connector_name: Connector, connector_account_details: admin::ConnectorAuthType }` (api_models/src/verify_connector.rs:5-9). C++ defines the typed shapes in the exclusive DTO: `Connector` enum with the full snake_case wire table from `common_enums/src/connector_enums.rs:29-235` (dto.cppm:27-198) and `connector_from_wire`/`connector_to_wire` validators (dto.cppm:363-389); `ConnectorAuthTypeValue` mirrors the internally-tagged `ConnectorAuthType` (`admin.rs:1582-1615`) with serde reflection (dto.cppm:390-443,445-463). The handler calls `serde::Ser::deserialize<VerifyConnectorRequest>` directly on the body (cppm:395-404) and validates the enum (cppm:406-416) — no generic JSON normalization, no raw-string wire DTO.
- **Output types (MATCH):** Upstream returns unit/StatusOk with no payload (core:23,60); C++ emits an empty body with `Status::OK` (cppm:474,517-529), no fabricated object and no request echo.
- **Auth/permissions (MATCH):** C++ resolves `auth_rules().rule_for("payment_connector_verify")` and `authorizer().authorize_route` (cppm:374-377); upstream passes `auth.profile.id` as `profile_id` (routes:28); C++ reads `auth_ctx.profile_id()` (cppm:386) and feeds it to core (unused there per core:22). No merchant-account lookup (removed).
- **Persistence (MATCH):** Upstream performs no DB write; connector resolution is config-based (core:24-30). C++ `get_connector_by_name` resolves against ConfigStore config (cppm:181-195) and no cache put, merchant-account write, or ephemeral fingerprint entry remains — the former C++-only cache write was removed.
- **Domain logic (MATCH):** `get_connector_by_name` (cppm:181-195) maps a miss to `IncorrectConnectorNameGiven` (cppm:34-40); `get_test_card_details` (cppm:113-129) + `FlowNotSupported` (cppm:45-53,431-434) gate unsupported connectors; `generate_card_from_details` parses the card and maps an invalid number to `InternalServerError` (cppm:89-105); `foreign_into` (cppm:151-161) builds the domain auth; `VerifyConnectorData {connector, connector_auth, card_details}` is built once (cppm:437-443); `get_payment_authorize_data` (cppm:217-222) and `get_router_data` (cppm:233-245) mirror the upstream types helpers; dispatch to `Stripe::verify` / `Paypal::get_access_token` (cppm:315-354) uses the structured payload, maps connector failures to `InvalidRequestData` (cppm:282-294), and returns `StatusOk`. The external PSP/network call is the sanctioned local in-process connector substitute (cppm:446-463).

## Not ported
None of the named upstream functions remain unported. The only deliberate boundary is the external dependency itself: the real `Stripe::verify` / `Paypal::get_access_token` PSP network calls (core:41,51; types/api/verify_connector.rs:185-191) are represented by the local in-process connector contract (`connector->verify()` / `connector->exchange_token()`, cppm:327-329,346-348), which is the stated "local in-process substitutes for external infra" policy. No C++-only route or C++-only persistence substitute remains on this route.

## Implementation disposition
Re-verified 2026-11-20 against `include/app/handlers/verify_connector.cppm` (534 lines) and `include/app/dto/verify_connector.cppm` (494 lines); line refs below are current.

- `core/verify_connector.rs:32-37` `get_test_card_details` + `FlowNotSupported` — IMPLEMENTED — `get_test_card_details` (cppm:113-129) is table-driven with the exact Stripe/Paypal cards (utils:39-52); nullopt → `flow_not_supported` `INVALID_REQUEST` (cppm:45-53,431-434).
- `core/verify_connector.rs:24-30` `ConnectorData::get_connector_by_name` + `IncorrectConnectorNameGiven` — IMPLEMENTED — `get_connector_by_name` (cppm:181-195) resolves the validated name against config and maps a miss to `incorrect_connector_name_given` (cppm:34-40), replacing name-string checks and generic NOT_FOUND errors.
- `core/verify_connector.rs:39-59` real `Stripe::verify` / `Paypal::get_access_token` with `foreign_into()` — IMPLEMENTED (in-process) — `stripe_verify` / `paypal_get_access_token` (cppm:315-354) carry the full upstream VerifyConnector algorithm (authorize data → access token → router data → build request → call connector → StatusOk/error). The PSP network hop is the sanctioned in-process substitute.
- `core/verify_connector.rs:41,51` `types::VerifyConnectorData {connector, connector_auth, card_details}` — IMPLEMENTED — structured `VerifyConnectorData` (cppm:199-204) built once at cppm:437-443 and passed to both dispatch helpers; `connector_auth` is the `foreign_into()` result (cppm:151-161).
- `core/verify_connector.rs:23,60` `RouterResponse<()>` / `StatusOk` empty response — IMPLEMENTED — `send_json(res, send, {}, true)` (cppm:474,517-529) emits an empty 200 with no echo.
- `routes/verify_connector.rs:36` `api_locking::LockAction::NotApplicable` — IMPLEMENTED — no lock acquired; `LockRegistry` deliberately unused (cppm:477-478).
- `routes/verify_connector.rs:12,18` `#[instrument]` / `Flow::VerifyPaymentConnector` — IMPLEMENTED — upstream flow marker mirrored by `record_flow("VerifyPaymentConnector")` + `counter("VERIFY_PAYMENT_CONNECTOR")` (cppm:469-471).
- `api_models/src/verify_connector.rs:6-8` typed request fields (`Connector` enum, `ConnectorAuthType`) — IMPLEMENTED — exclusive DTO rewrite (dto.cppm:27-198,363-477) + direct typed `deserialize` and enum validation (cppm:395-416); the former untyped generic decode and `VerifyConnectorRequestWire` were removed.
- `core/verify_connector.rs` error mapping — IMPLEMENTED — `IncorrectConnectorNameGiven` (cppm:34-40), `FlowNotSupported` (cppm:45-53), `InvalidRequestData` (cppm:58-63), `InternalServerError` (cppm:66-70) with upstream names, HTTP statuses and messages from `api_error_response.rs:532-638`.

## Shared contract requests
- `app_connector`: add a public `ConnectorRouter::select_by_name(name)` (a local `ConnectorData::get_connector_by_name` equivalent) backed by `state.conf.connectors`-style config. Today `get_connector_by_name` (cppm:181-195) stands in with `ConfigStore`; a shared select-by-name would remove that stand-in.
- `app_crypto` / card utilities: expose a `cards::CardNumber`-equivalent parser so `generate_card_from_details` (cppm:89-105) can map an invalid card number to `InternalServerError` with full Luhn/format validation instead of the current digits-only in-process check.
- `app_model` (shared DTO promotion): if any other domain needs the typed `Connector` enum / `ConnectorAuthType` wire shapes, promote them from the exclusive `app_dto:verify_connector` module to a shared model; currently they are module-local to avoid touching non-exclusive files.
- `app_connector` SDK integration: a real `Stripe::verify` / `Paypal::get_access_token` connector-integration interface (build_request + call_connector_api + get_error_response) would replace the in-process connector substitute. This is the external PSP dependency boundary.

## Files changed
- `include/app/handlers/verify_connector.cppm` — rewrote; added local helpers for every named upstream function, typed decode, exact error mappings; removed the generic wire DTO, merchant-account lookup and ephemeral cache write.
- `include/app/dto/verify_connector.cppm` — exclusive DTO; added typed `Connector` enum + snake_case wire table + validators, structured `ConnectorAuthTypeValue`, serde reflection.
- `docs/hyperswitch-parity/verify_connector.md` — this report; all rows moved to MATCH with upstream+C++ file:line evidence.
