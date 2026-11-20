# Parity: telephone_order

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/telephone_order.rs`, wired in `docs/hyperswitch/crates/router/src/routes/app.rs:2853-2862` (v1 scope `/telephone_order`, resource `/{merchant_id}/{provider}`, POST)
- Upstream core: `docs/hyperswitch/crates/router/src/core/telephone_order.rs` (664 lines); adapter `docs/hyperswitch/crates/router/src/core/telephone_order/twilio.rs` (153 lines); API types `api_models/telephone_order/twilio.rs` (`TwilioGenericPayRequest`/`TwilioGenericPayResponse`)
- C++: include/app/handlers/telephone_order.cppm
- Checked: 2026-11-20 (re-verified against current handler, 870 lines)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Both expose POST `/telephone_order/{merchant_id}/{provider}` (`app.rs:2856-2860`; `cppm:718-733`, route `:724-732`). v1 + `oltp` gated upstream; C++ still unconditional mount. |
| Input types | PARTIAL | Upstream typed `TwilioGenericPayRequest` w/ validation/masking (`twilio.rs:19-24,135-153`); C++ now deserializes into local typed `TwilioPayRequestWire` (`cppm:27-70,419`, serde `:774-797`) with card/amount/currency/tokenize/bank guards (`:483-524`). Local wire substitute — no upstream DTO or masking. |
| Output types | PARTIAL | Upstream `TwilioGenericPayResponse` + `ResponseParameters` map (`twilio.rs:119-128`; `core/telephone_order.rs:621-664`); C++ local `TwilioPayResponseWire` (`:72-88`) w/ `parameters` map (`:558-561,661-702`). Local wire shape, not `api_models` type. |
| Auth/permissions | PARTIAL | Upstream API-key Basic auth, merchant from route, `resolve_profile_id` (`core/telephone_order.rs:170-181,510-519`); C++ auth_rules rule + token auth (`:382-388`), now has `PROFILE_NOT_FOUND` on profile mismatch (`:526-542`) but from header `auth_ctx.profile_id`, no repository lookup. |
| Persistence | PARTIAL | Upstream payments_core create+confirm + `api_locking::Hold` (`core/telephone_order.rs:227-248,315-332`); C++ now takes a local lock (`:593-602,687`), duplicate check + `sync()` replay (`:604-622`), `save`/`update_payment` (`:670-685`). No state machine. |
| Domain logic | PARTIAL | 3-way outcome (`:315-329,689-713`) plus duplicate replay, transaction-id idempotency, error classifier, amount parser and guards now present as documented local substitutes (`:475-481,604-622,311-346,216-309,483-524`). Still no real `payments_core`/`extract_id`/SHA256 classifier. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST /telephone_order/{merchant_id}/{provider} | `telephone_order_charge` (`routes/telephone_order.rs:19-36`) → generic `charge_with::<TwilioGenericPay>` (`:38-91`) | `handle_telephone_order_charge` (`cppm:376-716`) | `TwilioGenericPayRequest` typed+masked (`twilio.rs:31`) vs local typed `TwilioPayRequestWire` + guards (`cppm:27-70,419,483-524`) | `TwilioGenericPayResponse` w/ params (`twilio.rs:32,119-128`) vs `TwilioPayResponseWire` w/ params map (`cppm:72-88,436-450,558-561,661-702`) | `ApiKeyBasicAuthWithMerchantIdFromRoute` + profile from params (`core/telephone_order.rs:170-181`) vs auth_rules+token auth + `PROFILE_NOT_FOUND` (`cppm:382-388,526-542`) | payments_core+DB+lock vs local lock + duplicate `sync()` + `save`/`update_payment` (`cppm:593-622,670-687`) | PARTIAL | `app.rs:2856-2860` / `routes/telephone_order.rs:19-91` / `cppm:718-733`; core `core/telephone_order.rs:209-310`; adapter `twilio.rs:29-129` |

## Not ported
- Duplicate-payment replay: `DuplicatePayment` collapses to `retrieve_payment` via `PSync` so a retry answers the original outcome instead of a decline (`core/telephone_order.rs:263-274,446-500`). C++ saves unconditionally, no duplicate path (`cppm:138-143`).
- API locking: `api_locking::LockAction::Hold` on the resolved payment id serializes in-flight retries (`core/telephone_order.rs:315-332`; resolved in `routes/telephone_order.rs:62`). C++ only records metrics (`cppm:82-83`).
- Idempotency from provider `transaction_id`: SHA-256, first 16 hex bytes, `pay_` prefix (`core/telephone_order.rs:185-191,426-442`; `twilio.rs:38-42`). C++ hashes the whole body with `KeyedHasher` (`cppm:94-95`) — different field order/serialization yields a different payment id.
- IVR-supplied `parameters.payment_id` takes precedence over the derived id; `parameters.profile_id`/`customer_id`/`metadata` parsed with `extract_id` and validated (`core/telephone_order.rs:185-191,336-401`). C++ ignores body-supplied ids and builds metadata from raw body (`cppm:94-95,142`).
- Profile handling: `resolve_profile_id` validates `parameters.profile_id` and maps a foreign profile to `PROFILE_NOT_FOUND` (`core/telephone_order.rs:358-370,510-519`). C++ reads profile/org from headers then discards (`cppm:87-89`).
- Twilio request mapping and guards: reject `method != Charge` (`TOKENIZE_UNSUPPORTED`) and bank-account fields (`BANK_ACCOUNT_UNSUPPORTED`), extract card, parse amount/currency via `StringMajorUnitForConnector`, build billing/description/`merchant_order_reference_id` (`twilio.rs:13-17,49-117,135-153`). C++ forwards the raw body with none of this (`cppm:99-103`).
- `PaymentsRequest` flags: `confirm=true`, `CaptureMethod::Automatic`, `AuthenticationType::NoThreeDs`, `PaymentChannel::TelephoneOrder`, `connector` left unset for merchant routing (`core/telephone_order.rs:101-116`). C++ sets only `operation("authorize")` (`cppm:99-103`) and picks a connector directly via `select(merchant_id)` (`cppm:109`), bypassing configured routing.
- Error classification: `classify_core_error` maps `ProfileNotFound`, `PreconditionFailed`, `InvalidRequestData`, `InvalidDataValue`, `MissingRequiredField`, `InvalidDataFormat` to typed `Failure` codes, else `PROCESSING_ERROR` (`core/telephone_order.rs:503-559`). C++ has no classifier; outcome is a status-string switch (`cppm:121-136`) with a single message prefix (`:148`).
- Typed outcome handling: `Outcome::{Paid,Declined,Indeterminate}` with connector `unified_code`/`error_code` on decline and loud logging on indeterminate (`core/telephone_order.rs:77-85,286-309,563-619`). C++ cannot surface connector decline codes; empty/unknown status is lumped into `PAYMENT_PENDING` (`cppm:122-136`).
- Provider response shape: `TwilioGenericPayResponse::success/failure` with a `ResponseParameters` map (`payment_id`, `status`, `amount_capturable`, optional `connector`/`connector_transaction_id`/`unified_code`/`amount_received`) (`twilio.rs:119-128`; `core/telephone_order.rs:621-664`). C++ returns a generic `ConnectorResponse` (`cppm:145-151`).
- Error responses: unknown provider → `InvalidRequestUrl` through `server_wrap` (`routes/telephone_order.rs:27-35`); undeserializable body → `InvalidDataFormat { field_name: "request body", ... }` (`:79-89`). C++ uses its own `INVALID_REQUEST`/`INVALID_JSON`/`EMPTY_REQUEST` builder (`cppm:41-50,73-80`).
- Instrumentation: `#[instrument(... Flow::TelephoneOrderCharge, payment_id)]` with the resolved id recorded on the span (`routes/telephone_order.rs:18,52-60`); C++ records only two metric counters (`cppm:82-83`).

## Implementation disposition
Supersedes the stale C++ column of `## Not ported` above: the current handler is 870 lines, not ~169. One line per original bullet.

- Duplicate-payment replay — SUBSTITUTED — `find_payment_by_string` duplicate check + `connector->sync()` as PSync substitute, original `status` reused (`cppm:604-609,618-622,643-651`); no `payments_core` PSync.
- API locking — IMPLEMENTED — `m_context->locks().acquire("telephone_order:"+payment_id, 30, ...)` with contention path and release (`cppm:593-602,636,687`).
- Idempotency from provider transaction_id — SUBSTITUTED — `KeyedHasher::hash_with_domain` head+tail, `pay_` + 32 hex (`cppm:475-481`, rationale `:457-459`); derived from `transaction_id`, not whole body, but not upstream SHA256[:16].
- IVR-supplied parameters.payment_id / profile / customer / metadata — SUBSTITUTED — `payment_id` precedence + `TelephoneOrderIdValidator` shape checks (`cppm:461-468,181-214`), profile/customer from parameters (`:526-552`), metadata JSON from parameters (`:554-556`); local validator instead of `extract_id`.
- Profile handling (PROFILE_NOT_FOUND) — SUBSTITUTED — supplied `profile_id` compared to `auth_ctx.profile_id`, mismatch → `PROFILE_NOT_FOUND` + metric (`cppm:526-542`); no repository `resolve_profile_id`.
- Twilio request mapping and guards — IMPLEMENTED — `TOKENIZE_UNSUPPORTED`/`BANK_ACCOUNT_UNSUPPORTED` guards (`:483-492`), card/expiry/cvv/amount/currency validation (`:493-524`), amount via local `MajorUnitAmountParser` (`:216-309`), billing/description/merchant_order_reference_id mapping (`:563-582`).
- PaymentsRequest flags — IMPLEMENTED — `confirm=true`, `capture_method=automatic`, `authentication_type=no_three_ds`, `payment_channel=telephone_order` on payload (`:572-576`); connector via routing-aware `select_for(connector_request)` (`:616`), no longer `select(merchant_id)`.
- Error classification — SUBSTITUTED — `TelephoneOrderClassifier::classify` maps to `INVALID_REQUEST`/`PROFILE_NOT_FOUND`/`PROCESSING_ERROR` (`:311-346,631-639`); coarser than upstream typed codes.
- Typed outcome handling — IMPLEMENTED — `ChargeOutcome::{Paid,Declined,Indeterminate}` + `classifier.outcome` (`:174-179,315-329`), decline code/message from decoded connector raw JSON (`:653-659,697-705`), indeterminate flow + `PAYMENT_PENDING` (`:707-712`).
- Provider response shape — SUBSTITUTED — `TwilioPayResponseWire` with `charge_id`/`error_code`/`error_message` + `parameters` map carrying payment_id/status/amount_capturable/connector/connector_transaction_id/unified_code/amount_received (`:72-88,436-450,558-561,661-702`); local wire shape, not `api_models` type.
- Error responses — SUBSTITUTED — own `AppErrorBuilder` still used, but upstream codes surfaced as `InvalidRequestUrl` (`:406-411`) and `InvalidDataFormat: request body` (`:419-424`) text; no `server_wrap`.
- Instrumentation — SUBSTITUTED — `metrics().record_flow` + counters (`:427-428,464,484,489,534,550,597,607-608,628,635,695,704,710-711`); no `#[instrument]` span with resolved payment_id.
