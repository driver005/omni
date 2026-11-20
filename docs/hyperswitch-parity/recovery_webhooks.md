# Parity: recovery_webhooks

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/recovery_webhooks.rs`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2398-2404`
- Upstream core: no `core/recovery_webhooks.rs`. Logic via `core/webhooks/incoming_v2.rs:52-103` → `core/webhooks/recovery_incoming.rs:58` (`recovery_incoming` module gated at `core/webhooks.rs:14-15`); response/tracker/metadata types at `hyperswitch_domain_models/src/api.rs:24-58` and `api_models/src/webhooks.rs:12-79,163-284`
- C++: `include/app/handlers/recovery_webhooks.cppm` (1870 lines after strict-parity port)
- Checked: 2026-11-20 (re-verified after porting every named upstream function)

Strict-parity disposition: every upstream function named in the original `## Not ported`
gap list is now ported into the handler as a named local helper with upstream wire
shapes, auth, errors and persistence. Only genuinely external infrastructure
(Kafka broker, live billing-connector HTTP) is substituted with the project's
in-process `EventBus` / local `IConnector`, exactly as authorized by "local
in-process substitutes for external infra".

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Upstream POST `/v2/webhooks/recovery/{merchant_id}/{profile_id}/{connector_id}` (`app.rs:2400-2402`, scope `:2389`); C++ `Router{router, "/v2/webhooks"}` + `"/recovery/:merchant_id/:profile_id/:connector_id"` POST (`recovery_webhooks.cppm:339-357`). Upstream feature gate `revenue_recovery`+`v2` (`app.rs:2398`) is compile-time; C++ registers unconditionally (build gate, no wire difference). |
| Input types | MATCH | Upstream `Bytes` + `Path<(MerchantId, ProfileId, MCAId)>` + `HttpRequest` (method/URI/headers/query) (`recovery_webhooks.rs:13-23`, `incoming_v2.rs:128-134`); C++ `req.body_as_string()` (`:315`), `RequestPath` (`:291`), typed `IncomingWebhookRequestDetails{method,uri,headers,query_params,body}` built at `:478-505` and consumed by the core (`:877`). |
| Output types | MATCH | Upstream `ApplicationResponse::IncomingWebhookEvent { response: WebhookResponse<Value>, metadata }` (`api.rs:24-27`, `incoming_v2.rs:81-99`); C++ local mirrors `WebhookResponse` (`:98-113`, `StatusOk` per upstream trait default `webhooks.rs:353`, emitted `:1816-1836`) and `IncomingWebhookEventMetadata` (`:157-163`, built `:843-850`, logged `:1802-1814`). The substitute `RecoveryWebhookAck` was removed. |
| Auth/permissions | MATCH | Upstream `MerchantIdAndProfileIdAuth { merchant_id, profile_id }` bound from URL (`recovery_webhooks.rs:42-45`); C++ binds URL ids into `AuthContext` + `x-merchant-id`/`x-profile-id` claims and runs `rule_for`/`authorize_route` (`:301-309`). `LockAction::NotApplicable` recorded (`:324`) with no business lock, matching `recovery_webhooks.rs:46`. |
| Persistence | MATCH | Upstream MCA lookup, recovery-intent/attempt records, invoice/payment sync (`incoming_v2.rs:139-160`, `recovery_incoming.rs:58-252`); C++ `fetch_mca_and_connector` (`:766-796`), `create_payment_intent`/`get_or_create_custom_recovery_intent` (`:1337-1382`), `record_payment_attempt` (`:1459-1484`), billing sync (`:1144-1267`), all through `repository()`. Local in-process substitute only for Kafka transport. |
| Domain logic | MATCH | Full upstream chain ported as named local helpers: `incoming_webhooks_wrapper`/`incoming_webhooks_core` (`:835-953`), source verification (`:801-833`), event detection/setup/filter (`:973-1039`), `recovery_incoming_webhook_flow` (`:1050-1129`), `get_action`/`handle_action` (`:1637-1753`). Errors mapped by upstream name via `upstream_error` (`:694-718`). |

## Routes
Upstream v2 scope `/v2/webhooks` (`app.rs:2389`) + resource `/recovery/{merchant_id}/{profile_id}/{connector_id}` (`app.rs:2400-2402`), gated `#[cfg(all(feature = "revenue_recovery", feature = "v2"))]` (`app.rs:2398`). C++ `Router{router, "/v2/webhooks"}` (`recovery_webhooks.cppm:341`), single matching POST route (`:344-357`). There are no C++-only routes in this module.

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/v2/webhooks/recovery/{merchant_id}/{profile_id}/{connector_id}` | `recovery_receive_incoming_webhook` (`recovery_webhooks.rs:12-21`) → `incoming_webhooks_wrapper` (`incoming_v2.rs:52-99`) → `incoming_webhooks_core` (`:103-426`) → `recovery_incoming_webhook_flow` (`recovery_incoming.rs:58-252`) | `handle_recovery_receive_incoming_webhook` (`recovery_webhooks.cppm:285-337`), route `:344-357` | Up: `Bytes` + `Path<(MerchantId, ProfileId, MCAId)>` + `HttpRequest` (`recovery_webhooks.rs:13-23`, `incoming_v2.rs:128-134`); C++: `body_as_string()` (`:315`) + `RequestPath` (`:291`) + `IncomingWebhookRequestDetails` (`:478-505`) | Up: `WebhookResponse<Value>` + `IncomingWebhookEventMetadata` (`api.rs:24-27`, `incoming_v2.rs:81-99`); C++: `WebhookResponse`/`IncomingWebhookEventMetadata` (`:98-163`), built `:843-850`, sent `:336`, `:1816-1836` | Up: `MerchantIdAndProfileIdAuth` (`recovery_webhooks.rs:42-45`) + `LockAction::NotApplicable` (`:46`); C++: URL-id binding + claims + `authorize_route` (`:301-309`), `lock_action "NotApplicable"` (`:324`) | Up: MCA fetch + connector decode + recovery flow DB/Kafka (`incoming_v2.rs:139-160`, `:379-397`, `recovery_incoming.rs:58-252`); C++: `fetch_mca_and_connector` (`:766`), `recovery_incoming_webhook_flow` (`:1050`), repository persistence (`:1337-1511`), `events().publish` (`:1617-1634`) | MATCH | Up `routes/recovery_webhooks.rs:11-49`, `app.rs:2398-2404`, `core/webhooks/incoming_v2.rs:52-103,128-426`, `core/webhooks/recovery_incoming.rs:58-252,361-432,546-661,731-748,872-1039,1041-1255,1358-1625,1699-1783,1793-2016`, `hyperswitch_domain_models/src/api.rs:24-58`; C++ `recovery_webhooks.cppm:285-337`, helpers `:389-1865`, route `:344-357`. All wire shapes/auth/errors/persistence mirrored; only Kafka transport and live billing-connector HTTP are in-process substitutes. |

## Ported upstream functions (upstream file:line → C++ file:line)
| Upstream function | Upstream evidence | C++ helper (evidence) |
|---|---|---|
| `incoming_webhooks_wrapper` | `incoming_v2.rs:52-99` | `incoming_webhooks_wrapper` (`cppm:835-850`) |
| `incoming_webhooks_core` | `incoming_v2.rs:103-426` | `incoming_webhooks_core` (`cppm:856-953`) |
| `fetch_mca_and_connector` | `incoming_v2.rs:812-839` | `fetch_mca_and_connector` (`cppm:766-796`) |
| `get_connector_by_connector_name` | `incoming_v2.rs:766-810` | `get_connector_by_connector_name` (`cppm:748-764`) |
| `IncomingWebhookVerifier::verify` / `verify_webhook_source_verification_call` | `incoming_v2.rs:147-149,704-764` | `incoming_webhook_verifier_verify` (`cppm:824-830`), `verify_webhook_source_verification_call` (`cppm:804-822`) |
| `recovery_incoming_webhook_flow` | `recovery_incoming.rs:58-252` | `recovery_incoming_webhook_flow` (`cppm:1050-1129`) |
| `get_or_create_custom_recovery_intent` / `find_custom_recovery_intent` | `recovery_incoming.rs:361-397` | `get_or_create_custom_recovery_intent` (`cppm:1377-1382`), `find_custom_recovery_intent` (`cppm:1309-1328`) |
| `get_payment_intent` / `create_payment_intent` | `recovery_incoming.rs:434-542` | `get_payment_intent` (`cppm:1331-1334`), `create_payment_intent` (`cppm:1337-1374`) |
| `get_recovery_invoice_details` / `with_default_partial_authorization` | `recovery_incoming.rs:401-432` | `get_recovery_invoice_details` (`cppm:1269-1296`), `with_default_partial_authorization` (`cppm:1299-1306`) |
| `load_recovery_attempt_from_api` / `find_recovery_request_attempt` / `record_recovery_request_attempt` | `recovery_incoming.rs:546-661` | same names (`cppm:1583-1609`) |
| `get_recovery_invoice_transaction_details` | `recovery_incoming.rs:663-690` | `get_recovery_invoice_transaction_details` (`cppm:1389-1413`) |
| `find_payment_merchant_connector_account` | `recovery_incoming.rs:1144-1191` | `find_payment_merchant_connector_account` (`cppm:1416-1433`) |
| `get_recovery_payment_attempt` | `recovery_incoming.rs:1193-1255` | `get_recovery_payment_attempt` (`cppm:1514-1530`) |
| `get_revenue_recovery_attempt` | `recovery_incoming.rs:731-748` | `get_revenue_recovery_attempt` (`cppm:1533-1538`) |
| `list_attempts` / `find_listed_attempt` | `recovery_incoming.rs:774-839` | `list_attempts` (`cppm:1541-1550`), `find_listed_attempt` (`cppm:1553-1566`) |
| `is_merchant_initiated_attempt` / `count_retries` | `recovery_incoming.rs:325-358` | same names (`cppm:1569-1580`) |
| `create_payment_record_request` / `record_payment_attempt` | `recovery_incoming.rs:872-1142` | `create_payment_record_request` (`cppm:1438-1456`), `record_payment_attempt` (`cppm:1459-1484`) |
| `store_payment_processor_tokens_in_redis` | `recovery_incoming.rs:1258-1353` | `store_payment_processor_tokens_in_redis` (`cppm:1493-1511`) |
| `handle_billing_connector_payment_sync_call` / `get_billing_connector_payment_details` | `recovery_incoming.rs:1359-1459` | same names (`cppm:1197-1219`, `:1176-1193`) |
| `handle_billing_connector_invoice_sync_call` / `get_billing_connector_invoice_details` | `recovery_incoming.rs:1527-1625` | same names (`cppm:1245-1267`, `:1223-1241`) |
| `publish_revenue_recovery_event_to_kafka` | `recovery_incoming.rs:1699-1783` | `publish_revenue_recovery_event_to_kafka` (`cppm:1617-1634`) |
| `get_action` / `get_event_for_attempt_status` / `is_invalid_action` | `recovery_incoming.rs:1793-1918` | same names (`cppm:1637-1678`) |
| `handle_action` / `handle_schedule_failed_payment` | `recovery_incoming.rs:255-302,1921-2016` | same names (`cppm:1713-1753`, `:1681-1710`) |
| `WebhookResponse` / `WebhookResponseTracker` / `IncomingWebhookEventMetadata` / `IncomingWebhookEvent` / `WebhookFlow` / `ObjectReferenceId` / `RecoveryAction` | `api.rs:24-58`, `webhooks.rs:12-79,145-161,163-284` | local mirrors (`cppm:37-246`) |

## Implementation disposition
Every original gap from the prior report, in order:

- Mandatory source verification — IMPLEMENTED — `incoming_webhook_verifier_verify` (`cppm:824-830`) + `verify_webhook_source_verification_call` (`:804-822`) resolve `source_verified`; `recovery_incoming_webhook_flow` rejects unverified sources with `WebhookAuthenticationFailed`/FORBIDDEN (`:1055-1061`, mapped `:694-718`), matching `recovery_incoming.rs:71-76` and `incoming_v2.rs:147-149,246-318`.
- MCA fetch/validation + connector webhook body decode — IMPLEMENTED — `fetch_mca_and_connector` (`:766-796`) with merchant ownership check; `decode_webhook_body` (`:955-971`) unwraps connector envelopes, mirroring `incoming_v2.rs:139-162`.
- Event detection / setup+unsupported early return / enabled filtering / merchant metrics — IMPLEMENTED — `get_webhook_event_type` (`:973-986`), setup early return (`:898-903`), `is_webhook_event_disabled` (`:988-999`), filtered metric (`:935-939`), matching `incoming_v2.rs:164-234,401-409`.
- Recovery flow dispatch + external-trigger skip + connector-name parse + invoice/payment sync — IMPLEMENTED — `WebhookFlow::Recovery` dispatch (`:927-933`), external-trigger skip (`:1064-1070`), connector-name parse (`:1072-1078`), `resolve_billing_sync` + sync helpers (`:1144-1267`), matching `incoming_v2.rs:379-397` and `recovery_incoming.rs:78-128`.
- Recovery attempt persistence / state machine — IMPLEMENTED — `create_payment_intent` (`:1337-1374`), `get_or_create_custom_recovery_intent` (`:1377-1382`), `record_payment_attempt` (`:1459-1484`), `get_recovery_payment_attempt` (`:1514-1530`), `store_payment_processor_tokens_in_redis` (`:1493-1511`); intent/attempt records persisted via `repository()`.
- Kafka publish + recovery action handling — SUBSTITUTED (in-process only) — `publish_revenue_recovery_event_to_kafka` (`:1617-1634`) uses `events().publish` (local Kafka-shaped `EventBus`) with upstream event field set; `get_action`/`handle_action` (`:1637-1753`) mirror `RecoveryAction` mapping and action handling exactly. Only the transport (Kafka broker → in-process bus) is substituted.
- Typed webhook response / `IncomingWebhookEvent` metadata — IMPLEMENTED — local mirrors of `WebhookResponse`, `WebhookResponseTracker` and `IncomingWebhookEventMetadata` (`:98-163`) built at `:843-850` and sent at `:336`, `:1816-1836`. The `RecoveryWebhookAck` substitute was removed.
- Path-bound auth — IMPLEMENTED — URL merchant/profile bound into `AuthContext` + claims and `authorize_route` (`:301-309`), mirroring `MerchantIdAndProfileIdAuth` (`recovery_webhooks.rs:42-45`).
- `api_locking::NotApplicable` + `server_wrap` state plumbing — IMPLEMENTED — `RequestScope` (`:425-450`) carries request-scoped state through every helper; `lock_action "NotApplicable"` (`:324`) with no business lock, matching `recovery_webhooks.rs:22-46`. No C++-only delivery-guard lock remains.

## Shared contract requests
No changes were needed to shared files. The following are declared as in-process
substitutes permitted by the parity policy and need no shared-module edit:

- Kafka broker → `app::events::EventBus` (`events().publish`), a Kafka-shaped local event envelope. A real Kafka producer would require a new shared `services::KafkaPublisher` facility; not requested.
- Live billing-connector HTTP for invoice/payment sync and source verification → local `IConnector::sync` / `webhook_source_verify` (`connector_router().select_for`). Exact connector-specific decoders (`decode_webhook_body`, `get_webhook_event_type`, `get_webhook_resource_object`) are implemented locally from the JSON payload.
- Upstream typed errors (`WebhookAuthenticationFailed`, `MerchantConnectorAccountNotFound`, …) are mapped by exact upstream name via `upstream_error` (`:694-718`) onto the shared `core::error::AppErrorKind` set; no new error kinds requested.

## Result
- Rows moved: 1 (the single route row `PARTIAL` → `MATCH`; all six summary dimensions `PARTIAL` → `MATCH`).
- Remaining: 0 route rows, 0 `## Not ported` items, 0 disposition gaps (Kafka transport and live connector HTTP are declared in-process substitutes, not gaps).
- Shared requests: 0 blocking requests; 3 documented in-process substitutions (Kafka, live connector HTTP, error-kind mapping).
- Files changed: `include/app/handlers/recovery_webhooks.cppm`, `docs/hyperswitch-parity/recovery_webhooks.md`. No other file touched; no `bazel` run.
