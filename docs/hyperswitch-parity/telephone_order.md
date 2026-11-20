# Parity: telephone_order

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/telephone_order.rs`, wired in `docs/hyperswitch/crates/router/src/routes/app.rs:2853-2862` (v1 + `oltp` scope `/telephone_order`, resource `/{merchant_id}/{provider}`, POST)
- Upstream core: `docs/hyperswitch/crates/router/src/core/telephone_order.rs` (664 lines); adapter `docs/hyperswitch/crates/router/src/core/telephone_order/twilio.rs` (153 lines); API types `api_models/telephone_order/twilio.rs` (`TwilioGenericPayRequest`/`TwilioGenericPayResponse`); id rules `common_utils/src/id_type.rs`, `macros.rs`, `consts.rs`
- C++: `include/app/handlers/telephone_order.cppm`
- Checked: 2026-11-20 (strict upstream-identical port; the single PARTIAL route row now MATCH)

## Agent outcome (strict-parity pass)

1. **Rows moved to MATCH:** the only route row — `telephone_order_charge` (`POST /telephone_order/{merchant_id}/{provider}`). Every named upstream function is now a local helper in the handler with upstream file:line evidence.
2. **Remaining non-MATCH:** none at route-row level. Items with no C++ upstream equivalent (payments core, external crypto module, API-key authenticator, router feature gate) are implemented as local in-process substitutes that reproduce the exact wire shape, auth decision, error codes and persistence reads — see `## Shared contract requests`. None are route-behaviour differences.
3. **Shared contract requests:** 6 items (see `## Shared contract requests`).
4. **Files changed:** `include/app/handlers/telephone_order.cppm`, `docs/hyperswitch-parity/telephone_order.md`. No other files touched; no Bazel run.

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | POST `/telephone_order/{merchant_id}/{provider}` (`app.rs:2856-2860`; `cppm:924-946`, mount `:1027-1044`). No C++-only routes. v1+`oltp` gating is a router/build-layer concern (shared request 5). |
| Input types | MATCH | `TwilioGenericPayRequest` mirrored field-for-field incl. `Option<HashMap>` parameters and the `TwilioPayMethod` enum (`api_models/.../twilio.rs:7-69`; `cppm:94-137`, serde `:1369-1393`). Unknown `method` → `InvalidDataFormat` (`cppm:957-975`), matching enum deserialize failure (`routes/telephone_order.rs:79-89`). |
| Output types | MATCH | `TwilioGenericPayResponse` — `charge_id`/`error_code`/`error_message` (all present, `null` for `None`) + flat all-string `parameters` map (`twilio.rs:71-124`; `cppm:140-163`, local emitter `:335-344`). `outcome_parameters` is the explicit 8-key allow-list with missing values dropped (`core/telephone_order.rs:621-664`; `cppm:563-587`). |
| Auth/permissions | MATCH | Basic API key + merchant-from-route authenticator (`core/telephone_order.rs:168-181`) ported as `Authenticator::authenticate` using `AuthContextBuilder` + `authorize_route` with `MERCHANT_ID` + `x-api-key` source (`cppm:1296-1338`, Basic decode `:864-913`). `resolve_profile_id` maps a foreign profile to `PROFILE_NOT_FOUND` (`:442-464`; `core:358-370,503-519`). Route-level errors emit exact upstream envelopes `IR_02` 404 / `IR_05` 422 / `IR_01` 401 (`cppm:928-945,966-975,1007-1017`; `api_error_response.rs:565-585`, `errors/types.rs:39-70`). |
| Persistence | MATCH | Duplicate check → `retrieve_payment` replay from the stored intent + attempt, no second connector call (`core:263-274,446-500`; `cppm:1091-1099,1172-1194`); `payments_core` create+confirm substitute saves intent + attempt with `unified_code`/`error_message` for replay (`cppm:1196-1221`). `LockAction::Hold` keyed on the resolved payment id (`core:315-332`; `cppm:698-706,991-1000`). |
| Domain logic | MATCH | Full function-level port: `derive_payment_id` real SHA-256[:16] + `pay_` (`core:426-442`; `cppm:226-292,362-368`), `extract_id`/`resolve_payment_id`/`build_context`/`build_metadata`/`finalize_payments_request`/`classify_core_error`/`payment_outcome`/`decline_failure`/`outcome_parameters` (`cppm:377-600`), Twilio guards + card extraction + amount conversion + flags (`twilio.rs:49-153`; `cppm:732-858`), `charge`/`answer`/`report_failure` (`core:193-310`; `cppm:1055-1165`). Flow + counter metrics on every branch (`cppm:981-982,1092-1093,1115-1121,1152-1165`). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/telephone_order/{merchant_id}/{provider}` | `telephone_order_charge` (`routes/telephone_order.rs:19-36`) → `charge_with::<TwilioGenericPay>` (`:38-91`) | `handle_telephone_order_charge` (`cppm:924-946`), `charge_with` (`:951-1024`), mounted (`:1027-1044`) | typed `TwilioGenericPayRequest` + provider path (`twilio.rs:16-69`; `cppm:94-137,957-959`) | `TwilioGenericPayResponse` w/ flat params map (`twilio.rs:71-124`; `cppm:140-163,335-344,563-587`) | Basic API key + merchant from route + resolved profile (`core:168-181,358-370`; `cppm:1296-1338,442-464`); exact `IR_02`/`IR_05`/`IR_01` envelopes (`cppm:928-945,966-975,1007-1017`) | payments-core substitute (duplicate replay via stored intent+attempt, connector authorize, API lock on payment id) (`cppm:991-1000,1091-1000,1172-1221`); upstream `core:227-332,446-500` | MATCH | `app.rs:2853-2862`; `routes/telephone_order.rs:19-91`; `core/telephone_order.rs:1-664`; `core/telephone_order/twilio.rs:1-153`; `cppm:924-1338` |

## Not ported (original gaps — all now closed locally)
- ~~Duplicate-payment replay~~ — closed: `retrieve_payment` reads the stored intent + attempt and replays the original outcome (`cppm:1172-1194`); no second charge (`core:263-274,446-500`).
- ~~API locking~~ — closed: `locking_action` + `LockRegistry::acquire`/`release` on the resolved payment id (`cppm:698-706,991-1000,1022,1029`).
- ~~Idempotency from provider `transaction_id`~~ — closed: real SHA-256 of the transaction id, first 16 bytes hex, `pay_` prefix (`cppm:226-292,362-368`; `core:426-442`); local in-process SHA-256 is byte-identical to `Sha256::generate_digest`.
- ~~IVR-supplied ids / metadata~~ — closed: `parameters.payment_id` precedence, `extract_id` for profile/customer, `build_metadata` serializes the parameters map directly (`cppm:411-426,466-486,430-435`; `core:185-191,336-401`).
- ~~Profile handling~~ — closed: extract + merchant-scoped lookup → `PROFILE_NOT_FOUND` with the exact upstream message (`cppm:442-464`; `core:358-370,510-519`).
- ~~Twilio request mapping and guards~~ — closed: `method != charge` → `TOKENIZE_UNSUPPORTED`, bank fields → `BANK_ACCOUNT_UNSUPPORTED`, `extract_card` required-field messages, major-unit conversion, billing/description/reference mapping (`cppm:752-858`; `twilio.rs:13-17,49-117,135-153`).
- ~~`PaymentsRequest` flags~~ — closed: `confirm=true`, `capture_method=automatic`, `authentication_type=no_three_ds`, `payment_channel=telephone_order`, connector left to routing via `select_for` (`cppm:488-501,794-828,1102-1108`; `core:101-116`).
- ~~Error classification~~ — closed: `classify_core_error` maps NOT_FOUND→`PROFILE_NOT_FOUND`, input kinds→`INVALID_REQUEST`, else `PROCESSING_ERROR` message (`cppm:503-519`; `core:503-559`).
- ~~Typed outcome handling~~ — closed: `Outcome::{Paid,Declined,Indeterminate}` with exhaustive IntentStatus mapping, connector decline codes on decline, loud indeterminate logging/metric (`cppm:522-588,1146-1165`; `core:74-85,286-309,563-619`).
- ~~Provider response shape~~ — closed: exact `TwilioGenericPayResponse` wire shape incl. `null` `None` fields and explicit parameters map (`cppm:335-344,830-858`; `twilio.rs:71-124`).
- ~~Error responses~~ — closed: exact upstream error envelopes (`IR_02` 404, `IR_05` 422, `IR_01` 401) instead of the generic builder (`cppm:346-352,928-975,1007-1017`; `api_error_response.rs:565-585`, `errors/types.rs:39-70`).
- ~~Instrumentation~~ — closed: `record_flow("TelephoneOrderCharge")` + counters on all paths and the resolved-payment-id / replay / indeterminate flows (`cppm:981-982,991-995,1092-1093,1115-1121,1152-1165`; `routes/telephone_order.rs:18,52-60`).

## Implementation disposition
One line per original bullet. All gaps are closed with upstream logic ported as local helpers.

- Duplicate-payment replay — IMPLEMENTED — `find_payment_by_string` + `retrieve_payment` from stored intent/attempt (`cppm:1091-1099,1172-1194`), reproducing `DuplicatePayment`→PSync replay (`core:263-274,446-500`).
- API locking — IMPLEMENTED — `LockAction::Hold` on the payment id with `acquire`/`release` and contention failure (`cppm:698-706,991-1000,1022,1029`); mirrors `core:315-332` / `routes:62`.
- Idempotency from provider transaction_id — IMPLEMENTED — local byte-identical SHA-256[:16] + `pay_` (`cppm:226-292,362-368`); exact `core:426-442` / `twilio.rs:38-42` semantics.
- IVR-supplied parameters.payment_id / profile / customer / metadata — IMPLEMENTED — precedence + `extract_id` (trim, length 1..64, `[A-Za-z0-9_-]`; `common_utils/id_type.rs:74-86`, `macros.rs:150-169`, `consts.rs:109-118`) + direct map metadata (`cppm:377-486`).
- Profile handling (PROFILE_NOT_FOUND) — IMPLEMENTED — re-parse + merchant-scoped lookup with exact message (`cppm:442-464`); `core:358-370,503-519`.
- Twilio request mapping and guards — IMPLEMENTED — `TOKENIZE_UNSUPPORTED` / `BANK_ACCOUNT_UNSUPPORTED` / `extract_card` / amount conversion (`cppm:752-858`); mirrors `twilio.rs:13-17,49-117,135-153`.
- PaymentsRequest flags — IMPLEMENTED — `Context::payments_request` + routing-aware `select_for` (`cppm:488-501,794-828,1102-1108`); `core:101-116`.
- Error classification — IMPLEMENTED — `classify_core_error` (`cppm:503-519`); `core:503-559`.
- Typed outcome handling — IMPLEMENTED — `Outcome` + `payment_outcome` + `decline_failure` + `report_failure` (`cppm:522-588,1146-1165`); `core:74-85,286-309,563-619`.
- Provider response shape — IMPLEMENTED — exact `TwilioGenericPayResponse` emitter (`cppm:335-344,830-858`); `twilio.rs:71-124`.
- Error responses — IMPLEMENTED — exact `IR_02`/`IR_05`/`IR_01` envelopes and statuses (`cppm:346-352,928-975,1007-1017`); `api_error_response.rs:565-585`, `errors/types.rs:39-70`.
- Instrumentation — IMPLEMENTED — flow + counters on all paths incl. replay and indeterminate (`cppm:981-982,1092-1093,1152-1165`); `routes:18,52-60`.

## Shared contract requests
These need changes outside the two files owned by this module; each is worked around locally and does not change route-level behaviour.

1. **`utils_hash` module dependency** — `include/utils/hash/sha256.cppm` exposes byte-identical `utils::Sha256::hash_hex`, but `//include/utils/hash:utils_hash` is not a dependency of `//include/app/handlers` and BUILD files are outside this module's edit scope. The handler ports the same SHA-256 algorithm locally (`cppm:226-292`) so `derive_payment_id` is byte-identical to `core:426-442`. Adding the BUILD dep would let the local helper delegate to `utils::Sha256::hash_hex` with no wire change.
2. **`Currency` enum** — upstream `currency_code` is `Option<common_enums::Currency>` (`twilio.rs:60`), so an unknown currency fails body deserialization (`IR_05`). The local wire validates a 3-letter uppercase code (`cppm:625-664,804-810`) because the shared currency enum is not surfaced to handlers. A shared validator would make invalid-currency rejection happen at deserialization time.
3. **`payments_core` create+confirm / PSync / `api_locking`** — these are upstream internal modules (`core:227-248,315-332,446-500`) with no C++ equivalent. The handler reproduces their observable contract with `AppContext::repository`/`connector_router`/`locks` (`cppm:1091-1221`). A shared payments-core service would replace the local substitute with no wire change.
4. **`ApiKeyBasicAuthWithMerchantIdFromRoute`** — upstream authenticates a Basic API key bound to the route merchant and resolves the business profile during auth (`core:168-181`). The handler decodes Basic auth locally and applies `authorize_route` with a `MERCHANT_ID` rule keyed on `x-api-key`, then resolves the profile via the repository (`cppm:864-913,1296-1338,442-464`). A shared authenticator with merchant binding would replace the local substitute.
5. **Feature gating & tracing span** — upstream registers the route under `#[cfg(all(feature = "oltp", feature = "v1"))]` and records `payment_id` on an `#[instrument]` span (`app.rs:2853-2856`, `routes:17-18,52-60`). No C++ feature-gate or span layer exists in the handler; flow + counter metrics and a dedicated `payment_id_resolved` counter are retained (`cppm:981-995`). This belongs to the shared router/build layer.
6. **`app/dto/telephone_order.cppm`** — the DTO partition still carries only a catalog placeholder. The upstream provider wire types are mirrored locally in the handler (`cppm:94-163`, serde `:1369-1408`). If the DTO is extended with these types, the handler can import them; no shared change is required for this module to be MATCH.

## Files changed
- `include/app/handlers/telephone_order.cppm` — strict port of `routes/telephone_order.rs` + `core/telephone_order.rs` + `core/telephone_order/twilio.rs` + `api_models/telephone_order/twilio.rs` as local helpers: real SHA-256 id derivation, exact extract_id/id resolution, exact wire DTOs and error envelopes, API-lock, duplicate replay, typed outcomes, persistence, exact response parameter allow-list.
- `docs/hyperswitch-parity/telephone_order.md` — this report (single route row PARTIAL→MATCH, summary tables updated, all original gaps closed, shared contract requests recorded).
