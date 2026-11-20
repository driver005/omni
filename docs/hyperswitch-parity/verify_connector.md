# Parity: verify_connector

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/verify_connector.rs` (`payment_connector_verify`); registered in `docs/hyperswitch/crates/router/src/routes/app.rs:2244-2245` under `web::scope("/account")`
- Upstream core: `docs/hyperswitch/crates/router/src/core/verify_connector.rs` (`verify_connector_credentials`)
- C++: include/app/handlers/verify_connector.cppm
- Checked: 2026-11-19
- Re-verified: 2026-11-20 (handler reworked; verdicts and disposition below reflect current `verify_connector.cppm`)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `POST /account/connectors/verify` both sides. |
| Input types | PARTIAL | Upstream typed `VerifyConnectorRequest` (Connector enum + `ConnectorAuthType`); C++ decodes generic JSON, normalizes into `VerifyConnectorRequestWire` with raw string fields (cppm:30-40, 198-230), no enum validation. |
| Output types | MATCH | Upstream `RouterResponse<()>` / StatusOk with no body; C++ emits empty `Status::OK` body, no fabricated object, no request echo (cppm:319-321, 364-376). |
| Auth/permissions | MATCH | JWT `MerchantConnectorWrite`/`CONNECTOR_WRITE`, `allow_connected=true`, `allow_platform=false`. |
| Persistence | PARTIAL | Upstream reads connector config + test-card details, no DB write; C++ does merchant-account lookup (cppm:252-256) + ephemeral cache put of credential fingerprint (cppm:309-313). |
| Domain logic | PARTIAL | Upstream real `Stripe::verify` / `Paypal::get_access_token`; C++ substitutes `TestCardDetailsLookup` (cppm:86-115, 241-248), structured `VerifyConnectorDataWire` (cppm:66-79, 268-274) and local connector `verify()`/`exchange_token()` (cppm:294-303); real connector SDK/external dependency absent. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/account/connectors/verify` | `payment_connector_verify` (routes:13-39) → `verify_connector_credentials` (core:19-67) | `handle_payment_connector_verify` (cppm:170-322), installed cppm:327-343 | `web::Json<VerifyConnectorRequest>` (routes:16); fields `connector_name: Connector`, `connector_account_details: ConnectorAuthType` (api_models:6-8) | `RouterResponse<()>` → `StatusOk` (core:23,60); C++ empty 200 (cppm:319-321) | `JWTAuth{MerchantConnectorWrite, allow_connected:true, allow_platform:false}` (routes:31-35); `api_locking::NotApplicable` (routes:36) | `ConnectorData::get_connector_by_name(state.conf.connectors,...)` (core:24-30); `Stripe::verify` / `Paypal::get_access_token` (core:41,51); Flow `VerifyPaymentConnector` (routes:12,18) | PARTIAL | Upstream: app.rs:2244-2245, routes:13-39, core:19-67; C++: cppm:170-322, 327-343. Path/verb, auth rule, profile-id source, connector-name gating and now the empty-200 output contract line up (cppm:319-321); input stays an untyped-string wire DTO (cppm:198-230) and credential verification uses the local connector contract (cppm:294-303). |

### Dimension evidence detail
- **Route paths/verbs (MATCH):** Upstream scope `/account` + resource `/connectors/verify` + `web::post()` (app.rs:2236,2244-2245); C++ `Router{router,"/account"}.add_route(..."/connectors/verify").post(...)` (cppm:327-343).
- **Input types (PARTIAL):** Upstream decodes `VerifyConnectorRequest` with enum-typed `connector_name` and structured `ConnectorAuthType` (api_models:6-8; routes:16). C++ rejects empty body (cppm:193-197), uses `decode_generic` (cppm:198) because the generated DTO wrappers lack serde support, then normalizes into `VerifyConnectorRequestWire` (cppm:30-40, 204-230) with `connector_name` and `connector_account_details` as raw strings. No connector-name enum validation; unsupported names rejected by `TestCardDetailsLookup` returning nullopt (cppm:241-248).
- **Output types (MATCH):** Upstream returns unit/StatusOk with no payload (core:23,60). C++ now emits an empty body with `Status::OK` (cppm:319-321, 364-376); the former 6-field fabricated object with request echo is gone.
- **Auth/permissions (MATCH):** C++ resolves `auth_rules().rule_for("payment_connector_verify")` and `authorizer().authorize_route` (cppm:176-182); upstream also passes `auth.profile.id` as `profile_id` (routes:28); C++ reads `auth_ctx.profile_id()` (cppm:190) with header-based merchant-id fallback (cppm:186-189).
- **Persistence (PARTIAL):** Upstream performs no DB write; connector resolution is config-based (core:24-30). C++ resolves merchant identity via `find_merchant_account_by_string` (cppm:252-256) and writes an ephemeral `verify_connector.attempt:<id>` → fingerprint entry to cache (cppm:309-313). Fingerprint uses `KeyedHasher` over `SecretString` credentials (cppm:261-266). No equivalent upstream cache write exists.
- **Domain logic (PARTIAL):** Upstream gates on `get_test_card_details` + `FlowNotSupported` (core:32-37) and dispatches `Connector::Stripe`/`Paypal` with `foreign_into()` + card details (core:39-59). C++ substitutes: `TestCardDetailsLookup` returns fixed Stripe/Paypal cards or nullopt → `flow not supported` error (cppm:86-115, 239-248); structured `VerifyConnectorDataWire {connector, connector_auth, card_details}` replaces the raw echo (cppm:66-79, 268-274); attempt id via `ids().payment_id("conn_verify_")` (cppm:258-259); connector selected via `connector_router().select_checked()` with `CONNECTOR_ERROR` on miss (cppm:283-292); dispatch to local `connector->verify()` / `connector->exchange_token()` (cppm:294-303) rather than real upstream SDK calls. Metrics/flow recorded locally (cppm:315-317).

## Not ported
- `core/verify_connector.rs:32-37` — `get_test_card_details(req.connector_name)` and `FlowNotSupported` when the connector has no test-card details; C++ only string-checks name (cppm:94-101), no test-card details logic.
- `core/verify_connector.rs:24-30` — `ConnectorData::get_connector_by_name` against `state.conf.connectors` with `IncorrectConnectorNameGiven` error mapping; C++ uses name-string checks and NOT_FOUND generic errors (cppm:86-101).
- `core/verify_connector.rs:39-59` — real `connector::Stripe::verify` and `connector::Paypal::get_access_token` with `connector_auth.foreign_into()` and `card_details` in `VerifyConnectorData`; C++ dispatches generic `authorize()` with raw payload (cppm:125-141).
- `core/verify_connector.rs:41,51` and `types::VerifyConnectorData` — structured `VerifyConnectorData {connector, connector_auth, card_details}` payload; no C++ equivalent (cppm:125-129 builds generic `ConnectorRequestBuilder`).
- `core/verify_connector.rs:23,60` — `RouterResponse<()>` / `StatusOk` empty response contract; C++ returns a fabricated 6-field JSON body with request echo (cppm:152-161).
- `routes/verify_connector.rs:36` — `api_locking::LockAction::NotApplicable`; no C++ locking concept present.
- `routes/verify_connector.rs:12,18` — `#[instrument]` with `Flow::VerifyPaymentConnector`; C++ substitutes local metrics calls (cppm:149-150), not upstream flow instrumentation.
- `api_models/src/verify_connector.rs:6-8` — typed request fields (`Connector` enum, `ConnectorAuthType`); C++ decodes untyped generic JSON (cppm:59-85).

## Implementation disposition
Re-verified 2026-11-20 against `include/app/handlers/verify_connector.cppm` (381 lines); line refs below are current.

- `get_test_card_details` + `FlowNotSupported` gate — SUBSTITUTED — `TestCardDetailsLookup` (cppm:86-115) returns fixed Stripe/Paypal cards, nullopt for all others → `flow not supported` INVALID_REQUEST (cppm:239-248); upstream table-driven card details (`core/verify_connector.rs:32-37`) not reproduced.
- `ConnectorData::get_connector_by_name` + `IncorrectConnectorNameGiven` — SUBSTITUTED — `connector_router().select_checked(merchant_id)` (cppm:283-292) maps miss/killed backend to `CONNECTOR_ERROR`; local router replaces `state.conf.connectors` name lookup (`core/verify_connector.rs:24-30`).
- Real `Stripe::verify` / `Paypal::get_access_token` with `foreign_into()` — SUBSTITUTED — local connector contract `connector->verify()` / `connector->exchange_token()` (cppm:294-303), operation chosen by name (cppm:276); `foreign_into()` stand-in is `SecretString` + `KeyedHasher` fingerprint (cppm:261-266). Real connector SDK/external service is an external dependency (`core/verify_connector.rs:39-59`).
- `VerifyConnectorData {connector, connector_auth, card_details}` — IMPLEMENTED — `VerifyConnectorDataWire` with those three fields (cppm:66-79) built and passed as one structured payload (cppm:268-274), replacing the old generic `ConnectorRequestBuilder` echo.
- `RouterResponse<()>` / `StatusOk` empty response — IMPLEMENTED — `send_json(res, send, {}, true)` emits an empty 200 with no echo (cppm:319-321, 364-376).
- `api_locking::LockAction::NotApplicable` — IMPLEMENTED — no lock acquired; `LockRegistry` deliberately unused on this route by design (cppm:324-325), matching upstream no-op (`routes/verify_connector.rs:36`).
- `#[instrument]` / `Flow::VerifyPaymentConnector` — SUBSTITUTED — local `metrics().record_flow("VerifyPaymentConnector")` + `counter("VERIFY_PAYMENT_CONNECTOR")` (cppm:315-317), not upstream tracing instrumentation (`routes/verify_connector.rs:12,18`).
- Typed request fields (`Connector` enum, `ConnectorAuthType`) — DEFERRED — generic JSON decode normalized into `VerifyConnectorRequestWire` with raw strings (cppm:30-40, 204-230); no enum validation or structured `ConnectorAuthType` decoding (`api_models/src/verify_connector.rs:6-8`). Generated DTO wrappers lack serde support, so an enum-typed decode needs DTO-layer changes outside this domain.
