# Parity: relay

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/relay.rs` (3 handlers); `docs/hyperswitch/crates/router/src/routes/webhooks.rs:48-83` (relay webhook); paths/verbs from `docs/hyperswitch/crates/router/src/routes/app.rs:1028-1035` (`web::scope("/relay")`) and `app.rs:2374-2382` (`/webhooks/relay`).
- Upstream core: `docs/hyperswitch/crates/router/src/core/relay.rs` (RelayInterface + 4 relay variants, flow decider, retrieve/sync, unreferenced-refund pipeline); `docs/hyperswitch/crates/router/src/core/webhooks/incoming.rs` (inbound webhook pipeline).
- C++: `include/app/handlers/relay.cppm`, `include/app/dto/relay.cppm`.
- Checked: 2026-11-20

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Upstream `POST ""`, `POST /unreferenced_refund`, `GET /{relay_id}`, `POST /webhooks/relay/{merchant_id}/{connector_id}` vs C++ `POST /relay/`, `POST /relay/unreferenced_refund`, `GET /relay/:relay_id`, `POST /webhooks/relay/:merchant_id/:connector_id` (nested trie at `relay.cppm:781-798`). No C++-only route remains in this module. |
| Input types | MATCH | Tagged `RelayData` variant required and validated per `from_relay_request` (`relay.cppm:301-358`); `connector_id`, `type`, `data.<variant>` decoded from the same wire shape (snake_case keys at `:1234-1257`). |
| Output types | MATCH | `RelayResponse` fields map 1:1; `data` is now an externally tagged object (`{"refund":{...}}`) via `tagged_relay_data`/`reconstruct_relay_data` (`relay.cppm:1260-1290`, DTO `:193-204`); `type` is snake_case (`relay.cppm:457`, `:589-591`). Error/ref fields populated with upstream messages. |
| Auth/permissions | MATCH | `HeaderAuth(ApiKeyAuth{allow_connected_scope_operation:true/false})` and webhook `MerchantIdAuth(merchant_id)` are encoded in the registry (`auth/registry.cppm:342,360,361,433`); handlers resolve those rules and overlay the webhook path merchant id (`relay.cppm:634-645`). |
| Persistence | MATCH | `relay_records` CRUD (`relay.cppm:151,383,499`) with refund mirror; connector reference id persisted into `refund_id` for connector-refund webhook lookups (`:177,:411`); process trackers for sync/refresh. |
| Domain logic | MATCH | Ported `relay_flow_decider` dispatch + `Validate` (`relay.cppm:301-358`), unreferenced pipeline support/key/tracker/connector path (`:104-186`), force-sync terminal-state gate (`:538`, `:1088-1108`), and the full inbound relay webhook pipeline (`:622-737`, helpers `:868-1179`). External connector HTTP is substituted by the local connector registry as permitted. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/relay` (`""` scope) | `routes::relay::relay` -> `core::relay::relay_flow_decider` (`relay.rs:51`, core `:670`) | `handle_relay` (`cppm:228`) | Up: `RelayRequest{type,data}` with `RelayRequestInner<T>::from_relay_request` + `Validate` (`core:133-149`, `:46-95`); C++: generic decode + required variant key + exact amount checks (`cppm:301-358`) | Up: `RelayResponse` from `RelayUpdate`; C++: same fields, tagged `data`, snake_case `type`, upstream error strings (`:414-462`) | Up: `ApiKeyAuth{connected_scope:true}`; C++ rule `relay` + v2-gated profile lookup (`:229-231`, `:270-274`) | Up: profile/MCA, `insert_relay`/`process_relay`/`update_relay`; C++: `relay_records` create/update + refund mirror + reference id + `dispatch` (`:367-415`) | MATCH | Up `relay.rs:51-81`, `core/relay.rs:670-820`, `api_models/relay.rs:8-32`; C++ `relay.cppm:228-462`, route `:741-752` |
| POST `/relay/unreferenced_refund` | `routes::relay::unreferenced_refund` -> `core::relay::relay_unreferenced_refund` (`relay.rs:14`, core `:1254`) | `handle_unreferenced_refund` (`cppm:30`) | Up: `UnreferencedRefundRequest` + amount>0 + dimension min/max (`core:1194-1252`); C++: typed DTO + same checks + `relay_payout_limits` superposition bounds (`:54-76`) | Up: `UnreferencedRefundResponse` w/ `raw_connector_response`; C++: same field set incl. `connector_reference_id`/`raw_connector_response` (`:197-224`) | Up: `ApiKeyAuth{connected_scope:false}`, required profile; C++ rule `unreferenced_refund` + profile resolution (`:37-97`) | Up: relay connector + access token + `call_connector_api` + insert/update; C++: support check with exact `Connector {} does not support unreferenced refund`, config/key store credentials, tracker, `relay_records` + connector `refund` (`:104-186`) | MATCH | Up `relay.rs:14-47`, `core/relay.rs:1046-1192`, `:1194-1295`; C++ `relay.cppm:30-224`, route `:753-762` |
| GET `/relay/{relay_id}` | `routes::relay::relay_retrieve` -> `core::relay::relay_retrieve` (`relay.rs:85`, core `:822`) | `handle_relay_retrieve` (`cppm:464`) | Up: `RelayRetrieveRequest{force_sync,id}`; C++: path/body id + `force_sync` query (`:466-478`) | Up: `RelayResponse::from`; C++: same shape, tagged `data`, snake_case `type`, sync error/ref fields (`:557-599`) | Up: `ApiKeyAuth{connected_scope:true}`, required profile; C++ rule `relay_retrieve` (`:454-456`) | Up: `find_relay_by_id` (404 `relay not found`), force-sync gateway refresh; C++: `find_relay_record`, `sync_relay_status` gated by `should_call_connector_*` terminal rule, tracker/store update (`:482-555`, `:1097-1140`) | MATCH | Up `relay.rs:85-119`, `core/relay.rs:822-957`, `:947-957`; C++ `relay.cppm:464-599`, route `:763-773` |
| POST `/webhooks/relay/{merchant_id}/{connector_id}` | `receive_incoming_relay_webhook` (`webhooks.rs:48`, app.rs `:2376-2380`) | `handle_incoming_relay_webhook` (`cppm:622`) | Up: raw webhook `Bytes` + path ids; C++: raw body verbatim, path ids via `webhook_path_ids` (`:850-865`) | Up: `WebhookResponse::StatusOk` (200, no body) + `IncomingWebhookEvent` metadata; C++: `send_ok` 200/empty + tracker metadata event (`:1145-1179`) | Up: `MerchantIdAuth(merchant_id)`; C++: rule `receive_incoming_relay_webhook` with path id overlay (`:634-645`) | Up: MCA validate, source verification, relay refund flow -> status update or force-sync; C++: all ported as local helpers (`:868-1142`) | MATCH | Up `webhooks.rs:48-83`, `app.rs:2374-2382`, `incoming.rs:78-119`, `:177-471`, `:541-803`, `:1550-1634`, `:1789-1825`, `:3557-3595`, `types/transformers.rs:506-519`; C++ `relay.cppm:622-737`, route `:781-798` |

## Not ported (external infra only)
- Upstream DB/Diesel interfaces (`insert_relay`/`update_relay`/`find_relay_by_id`), processor key store, profiles store and the accounts store are represented by `app::store::relay_records`, `app::keystore`, `config_store` and `superposition()`.
- Connector-specific webhook decoders/gateway and `call_connector_api` HTTP transport are replaced by `decode_relay_webhook` + `app::connector::IConnector::{webhook_source_verify,sync,refund,capture,cancel,authorize}` and process trackers.
- Upstream Rust type machinery (`RelayInterface` trait + four `RelayRequestInner<T>` impls, domain `relay::Relay`, `ForeignTryFrom`) is expressed as local helpers/enums in the handler; the wire-visible validation, ids, statuses, data and errors are identical.
- v1/v2 `#[cfg]` compile-time branches are runtime `feature_enabled("v2")`/`("oltp")` checks (`relay.cppm:89-93`, `:270-274`, `:412-415`).
- `server_wrap` lock semantics use `LockAction::NotApplicable`; the C++ handler records the flow metric and consults the lock registry without acquiring a lock (`relay.cppm:661-663`, `:1429-1435`).

## Implementation disposition
- `relay_flow_decider` type dispatch — IMPLEMENTED — `relay_kind` + exact per-variant validation and `from_relay_request`-equivalent variant requirement (`relay.cppm:301-358`, `:1201-1212`, `:1234-1257`).
- Per-variant `Validate` — IMPLEMENTED — upstream strings "Amount should be greater than 0" and "Capture Amount should be less than or equal to Authorized Amount" (`:318-352`); unreferenced "Amount must be greater than 0" (`:55-59`).
- Typed `RelayRequestInner<T>` / `RelayInterface` — SUBSTITUTED (local helpers) — generic decode + `dispatch` + inline response building (`:301-358`, `:1249-1290`).
- Domain `relay::Relay` construction — SUBSTITUTED — `relay_records::RelayRecord` with `internal_{uuid_v7}` resource id and "Created" status (`:366-383`).
- `insert_relay`/`update_relay`/`find_relay_by_id` + processor key store — SUBSTITUTED — relay record repository with connector reference id mirrored into `refund_id` (`:151`, `:177`, `:383`, `:411`, `:499`).
- Profile + MCA lookups — IMPLEMENTED — repository lookup plus v2-gated `find_profile_by_string` with local AppError equivalents (`:79-97`, `:261-281`).
- `relay_retrieve` force-sync — IMPLEMENTED — `should_call_connector_*` terminal gate + `sync_relay_status` with tracker and store refresh (`:538`, `:1099-1142`).
- Unreferenced-refund pipeline — IMPLEMENTED as local substitute — support check with exact error string, config base url + key store token, tracker, connector `refund` (`:104-186`).
- Min/max payout bounds — SUBSTITUTED — `payout_bounds` via `superposition().resolve` (`:61-76`, `:1397-1410`).
- `RelayResponse::from` — IMPLEMENTED — tagged `data`, snake_case `type`, error/ref fields (`:414-462`, `:557-599`, `:1260-1291`).
- Relay webhook scope — IMPLEMENTED — route and full pipeline (`webhooks.rs:48-83`; `relay.cppm:622-737`, `:781-798`, helpers `:868-1179`).
- v2 / oltp branches — IMPLEMENTED — config-backed `feature_enabled` (`:89-93`, `:270-274`, `:412-415`).
- Flow/lock semantics — IMPLEMENTED — `record_flow` + non-acquiring lock check (`:661-663`, `:1429-1435`).

## Shared contract requests
- `include/app/handlers/webhooks.cppm` registers a parallel `POST /webhooks/relay/:merchant_id/:connector_id` at lines 351-361 (`handle_receive_incoming_relay_webhook`, generic `process_webhook` substitute) plus a C++-only `POST /webhooks/receive_incoming_relay_webhook` at lines 362-372. That module is not mine to edit: please remove the duplicate relay webhook route (and the C++-only one) so the faithful handler in `relay.cppm:622-737` is the single owner of the upstream path.
- `include/app/auth/registry.cppm:342` maps `receive_incoming_relay_webhook` to `AuthScheme::HEADER_MERCHANT` + `AppPermission::WEBHOOK_WRITE`; no change needed, but note the relay webhook handler relies on that rule being present.
- `app::store::relay_records::RelayRecord` has no `connector_reference_id` column; the connector reference id is mirrored into `refund_id` locally. A dedicated column would let `find_relay_by_profile_id_connector_reference_id` be represented exactly.

## Files changed
- `include/app/handlers/relay.cppm` — added `handle_incoming_relay_webhook` and its local pipeline helpers; aligned input validation, error strings, wire shapes (tagged `data`, snake_case `type`), persistence of connector reference id, and force-sync terminal gate; registered the webhook route.
- `include/app/dto/relay.cppm` — `RelayResponse::data` changed from a bare `RelayData` enum to `std::optional<serde::Value>` so it serializes as upstream's externally tagged object. This DTO is consumed only by `include/app/handlers/relay.cppm`.
- `docs/hyperswitch-parity/relay.md` — this report.
