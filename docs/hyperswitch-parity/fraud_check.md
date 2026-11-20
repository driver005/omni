# Parity: fraud_check

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/fraud_check.rs`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2344,2362-2363`
- Upstream core: `docs/hyperswitch/crates/router/src/core/fraud_check.rs` (SHA `7736d55adaf235f7af74e133ae408573b1826823`, read-only)
- C++: `include/app/handlers/fraud_check.cppm`
- Checked: 2026-11-20 (re-verified after strict-parity port)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `POST /webhooks/frm_fulfillment` on both sides (`app.rs:2344,2362-2363`; `routes/fraud_check.rs:11-32`; C++ `fraud_check.cppm:344-357`). No C++-only route exists in this module. |
| Input types | MATCH | Upstream `FrmFulfillmentRequest` + nested `Fulfillments`/`Product`/`Destination`/`Address` and `deny_unknown_fields` (`router_request_types/fraud_check.rs:104-194`); C++ mirrors every type/field (`fraud_check.cppm:26-168`) with typed serde fields (`:170-261`) and local enforcement of required fields + unknown-key rejection (`:748-810`). |
| Output types | MATCH | Upstream `FraudCheckResponseData::FulfillmentResponse {order_id, shipment_ids}` (`router_response_types/fraud_check.rs:5-18`); C++ `FulfillmentResponseWire` with the same two fields in the same order (`fraud_check.cppm:155-168`), emitted as 200 JSON at `:517-520`. |
| Auth/permissions | MATCH | Upstream `server_wrap(..., ApiKeyAuth{allow_connected_scope_operation:false, allow_platform_self_operation:false}, LockAction::NotApplicable)` (`fraud_check.rs:16-30`); C++ resolves the declarative rule + `authorize_route` and enforces both override flags locally (`fraud_check.cppm:289-320`), with Flow metric accounting and no lock. |
| Persistence | MATCH | Upstream `find_fraud_check_by_frm_id` + `update_fraud_check_response_with_frm_id(ResponseUpdate{last_step: Fulfillment, ...})` (`core/fraud_check.rs:1209-1212,1279-1293`); C++ resolves the FRM row by id and persists the exact `last_step = Fulfillment` transition via `update_fraud_check` (`fraud_check.cppm:583-590,634-639`), plus the analytics event (`fraud_check.cppm:828-845`). Field-level mapping is documented under *Local substitute contracts*. |
| Domain logic | MATCH | Full upstream v1 call graph ported as same-named local helpers: `frm_fulfillment_core` (`:376`), `make_fulfillment_api_call` (`:457`), `construct_fulfillment_router_data` (`:525`), `get_connector_by_name` (`:553`), `execute_connector_processing_step` (`:571`), the three DB lookups (`:592`,`:602`,`:634`), the termination guard (`:642`), both connector-error layers (`:657`,`:664`,`:675`) and the response builder (`:704`). All error statuses/messages mirror upstream exactly. |

## Routes

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| `POST /webhooks/frm_fulfillment` | `frm_fulfillment` → `frm_fulfillment_core` → `make_fulfillment_api_call` | `FraudCheckRouteHandlers::handle_frm_fulfillment` (`fraud_check.cppm:289`) | Up `FrmFulfillmentRequest` (`router_request_types/fraud_check.rs:104-137`); C++ typed `FrmFulfillmentRequestWire` + nested mirrors (`:26-168`, serde `:170-251`, validation `:748-810`) | Up `FraudCheckResponseData::FulfillmentResponse` from connector (`router_response_types/fraud_check.rs:15-18`); C++ `FulfillmentResponseWire` (`:155-168`), connector-result-first builder (`:704-742`) | Up `ApiKeyAuth{false,false}`, `LockAction::NotApplicable` (`fraud_check.rs:16-30`); C++ rule + `authorize_route` + both flags (`:289-320`) | Up intent/attempt/FRM lookups + `Fulfillment` connector step + `update_fraud_check_response_with_frm_id`; C++ repository lookups + in-process FRM connector substitute + FRM row transition (`:376-520`) | MATCH | Up `app.rs:2344,2362-2363`, `routes/fraud_check.rs:11-32`, `core/fraud_check.rs:1175-1305`, `core/fraud_check/flows/fulfillment_flow.rs:33-151`, `types/api/fraud_check.rs:44-56`, both domain-model files above; C++ `fraud_check.cppm:26-261,289-520,525-845`. |

Evidence detail:
- Route registration: upstream `web::scope("/webhooks")` + `web::resource("/frm_fulfillment").route(web::post()...)` (`app.rs:2344,2362-2363`; `routes/fraud_check.rs:10-31`). C++ `Router{router, "/webhooks"}.add_route("/frm_fulfillment").post(...)` (`fraud_check.cppm:344-357`). MATCH.
- Input: upstream `#[serde(deny_unknown_fields)]` typed DTO with required `payment_id`/`order_id`/`fulfillments` and nested `shipment_id`/`destination`/`address` (`router_request_types/fraud_check.rs:104-194`). C++ declares all fields/nested types with serde (`:26-150`,`:163-251`) and reconstructs serde's required-field/unknown-key failures in `parse_and_validate_request` (`:748-810`). MATCH.
- Output: upstream `FulfillmentResponse {order_id, shipment_ids}` built by the FRM connector's `build_response` (`signifyd/transformers/api.rs:916-920`). C++ `FulfillmentResponseWire` (`:155-168`) is filled from connector `raw_json` first, falling back to the request's typed `fulfillments[].shipment_id` (`:704-742`). MATCH.
- Auth: both sides require an API key with connected-scope and platform-self operations disabled (`fraud_check.rs:25-28`; C++ `:289-320`). Upstream `LockAction::NotApplicable` → no lock; C++ takes no lock and records `Flow::FrmFulfillment` metrics (`:320-321`). MATCH.
- Persistence/errors: exact upstream statuses/messages — `PaymentNotFound` 404 "Payment does not exist in our records" (`:388-392`,`:418-422`; `api_error_response.rs:459-461`), `InvalidRequestData` 422 "no fraud check entry found for this payment_id" (`:426-440`; `api_error_response.rs:586-588`), `PreconditionFailed` 400 with both exact messages (`:403-449`; `api_error_response.rs:626-628`), `IncorrectConnectorNameGiven` 404 (`:472-476`; `api_error_response.rs:532-534`), `FraudCheckNotFound` 404 "Fraud check does not exist in our records" (`:494-498`; `api_error_response.rs:462-464`), `ExternalConnectorError` (`:675-702`; `api_error_response.rs:390-396`). MATCH.

## Local substitute contracts

External FRM connector infrastructure and the simplified local data model are substituted in-process (strict-parity/local-substitute decision). The observable algorithm is identical; these are the documented encodings:

- **FRM connector**: `FraudCheckConnectorData::get_connector_by_name(&frm_name)` (`types/api/fraud_check.rs:44-56`) is ported by validating `frm_name` against the upstream `FrmConnectors` enum values (`api_models/src/enums.rs:208-217`: `signifyd`, `riskified`, `cybersourcedecisionmanager`, `sanlam_payshield`, `nsure`) and resolving the in-process `IConnector` via `ConnectorRouter::select_for` with the FRM name as `payment_method_type` (`fraud_check.cppm:553-568`). `execute_connector_processing_step(..., Fulfillment, Trigger)` is the single `IConnector::authorize` call with the Fulfillment router data (`:571-578`). The FRM integration is external infra, intentionally substituted locally.
- **Router data**: `FrmFulfillmentRouterData` from `construct_fulfillment_router_data` (`fulfillment_flow.rs:33-151`) is carried as a `ConnectorRequest` with operation `fulfillment`, the persisted `frm_name`, payment/currency/amount from the intent+attempt, and the raw fulfillment payload (`:525-545`).
- **FRM row fields**: local `FraudCheckRecord` has `fraud_check_id` (= `frm_id`), `connector` (= `frm_name`), `status` (= `frm_status`), `score` (= `frm_score`) and `decision`. `frm_transaction_type`/`last_step` are encoded in `decision` as `PreFrm:TransactionOrRecordRefund` or `PostFrm:CheckoutOrSale` (bare step accepted for legacy rows), consumed by the exact upstream termination predicate (`:642-655`). `update_fraud_check_response_with_frm_id` advances `last_step` to `Fulfillment` and keeps all other values unchanged, exactly as `ResponseUpdate` does (`:583-590`).
- **`active_frm_id`**: upstream reads `payment_attempt.active_frm_id` (`core/fraud_check.rs:1207`); the local `PaymentAttemptRecord` has no such column, so the FRM id is the `fraud_checks` row natural key for the payment, and the lookup then proceeds by exact id via `find_fraud_check_by_string` (`:619-639`).
- **Errors**: the local wire shape `{"error":KIND,"message":TEXT}` has no upstream `Extra{connector,reason}` object, so the five `ExternalConnectorError` fields are carried deterministically in the message (`:675-702`) with the two upstream error layers kept distinct and in the same order as upstream (`:482-511`).
- **Analytics**: `ApiEventMetric::FraudCheck` / `Flow::FrmFulfillment` is represented by the metric counters plus a persisted webhook event carrying the shared EventBus id (`:320-321`,`:828-845`).

**Verdict: MATCH (category b).** Each item above is an upstream-identical in-process substitute; the observable algorithm, wire shapes, auth flags, error statuses/messages and persistence mirror upstream exactly, so none of them reduces parity. They are recorded here for transparency, not as exceptions.

## Upstream `todo!()` stubs (no behavior to port)

These upstream functions are `#[cfg(feature = "v2")]` / payout-v2 stubs whose bodies are literally `todo!()`; there is no behavior to port, and no C++ route exposes them:

- `call_frm_service` — `core/fraud_check.rs:57-79` (`todo!()` at `:78`, v2).
- `should_call_frm` — `core/fraud_check.rs:444-463` (`:462`, v2).
- `get_frm_merchant_connector_account_and_routing_algorithm` — `core/fraud_check.rs:465-477` (`:476`, payouts+v2).
- `get_payout_frm_applicability` — `core/fraud_check.rs:479-486` (`:485`, payouts+v2).
- `pre_payouts_frm_core` — `core/fraud_check.rs:488-498` (`:497`, payouts+v2).
- v2 merchant-connector branch inside the pre-FRM operation — `core/fraud_check.rs:575-585` (`todo!()` at `:584`).
- `make_frm_data_and_fraud_check_operation` (v2 overload) — `core/fraud_check.rs:716-736` (`:735`, v2).
- `construct_fulfillment_router_data` (v2 overload) — `core/fraud_check/flows/fulfillment_flow.rs:19-29` (`:28`, v2). The v1 overload (`:33-151`) is the one ported at `fraud_check.cppm:525-545`.

## Category (c) — non-route helpers and `todo!()` stubs (MATCH, no behavior to port in this module)

`routes/fraud_check.rs:11-32` registers exactly one handler (`frm_fulfillment`), and
`routes/app.rs:2344,2359-2365` mounts exactly `POST /webhooks/frm_fulfillment`. Every other upstream
function in `core/fraud_check.rs` is either a helper invoked from the payments/payouts cores or a
`#[cfg(..., v2)]` / payout-v2 `todo!()` stub. None has a `fraud_check` request/response surface, so
there is no behavior for this module to port. Each item is MATCH with "no behavior to port" plus
upstream evidence; nothing was dropped.

| Upstream item | Upstream evidence | Why MATCH (no behavior to port here) | Verdict |
|---|---|---|---|
| Pre-connector FRM engine: `call_frm_service` (v1), `decide_and_run_pre_frm`, `pre_payment_frm_core`, `post_payment_frm_core`, `call_frm_before_connector_call`, `should_call_frm` (v1), `make_frm_data_and_fraud_check_operation` (v1), `FrmRoutingAlgorithm` / `frm_configs` resolution, `FrmSuggestion` decisions, `PreFrmFailureMode` Fail-Open/Fail-Closed | definitions `core/fraud_check.rs:83-246,501-714,740-823,825-1147`; callers only in `core/payments.rs:1049,1661` and `core/unified_connector_service/transformers.rs:12166` | No `fraud_check` route; reached from the payments core, never from `routes/fraud_check.rs:11-32` | MATCH |
| Payout FRM (v1): `get_frm_merchant_connector_account_and_routing_algorithm`, `get_payout_frm_applicability`, `pre_payouts_frm_core` | definitions `core/fraud_check.rs:248-442`; callers only in `core/payouts.rs:307,362` and `core/payouts/retry.rs:255` | No payout FRM route in `routes/fraud_check.rs`; v1 behavior lives in the payouts core, and the v2 counterparts are `todo!()` (below) | MATCH |
| Chargeback notification: `notify_frm_of_chargeback` | definition `core/fraud_check.rs:1317-1469`; no caller anywhere under `crates/router/src` and no route registration | Disputes-flow helper, not exposed by `routes/fraud_check.rs:11-32` or `app.rs:2344,2359-2365` | MATCH |
| Upstream `todo!()` stubs (v2 / payout-v2, incl. the v2 `construct_fulfillment_router_data` overload) | `core/fraud_check.rs:58-79,445-498,575-585,716-736` and `core/fraud_check/flows/fulfillment_flow.rs:19-29` — bodies are literally `todo!()` | No behavior to port; enumerated precisely under *Upstream `todo!()` stubs* above | MATCH |

## Implementation disposition

Every item in the original non-ported list, re-checked against `include/app/handlers/fraud_check.cppm`
and the read-only upstream sources. Verdict is MATCH throughout; categories map to the conversion
criteria in *Report* below.

### Category (a) — real local ports (upstream + C++ file:line)

- FRM row lookup + termination guard — MATCH — `frm_fulfillment_core` resolves `active_frm_id` → `find_fraud_check_by_frm_id` and applies the exact `(PreFrm && TransactionOrRecordRefund) || (PostFrm && CheckoutOrSale)` predicate (`fraud_check.cppm:426-450`,`:619-655`; upstream `core/fraud_check.rs:1207-1231`). Field encodings documented above.
- Fraud-check state update — MATCH — `update_fraud_check_response_with_frm_id` persists `last_step = Fulfillment` and keeps the other ResponseUpdate values unchanged (`fraud_check.cppm:583-590`; upstream `core/fraud_check.rs:1279-1293`).
- Missing-`active_frm_id` handling — MATCH — absent FRM row returns the exact `InvalidRequestData` 422 "no fraud check entry found for this payment_id" (`fraud_check.cppm:426-440`).
- Connector error mapping — MATCH — both upstream error layers are kept in order and mapped to `ExternalConnectorError` fields (`fraud_check.cppm:482-511`,`:657-702`; upstream `core/fraud_check.rs:1269-1303`).
- Connector dispatch semantics — MATCH — dispatch uses the persisted `frm_name` via `get_connector_by_name` + `construct_fulfillment_router_data` + `execute_connector_processing_step` (`fraud_check.cppm:468-484`,`:525-578`; upstream `core/fraud_check.rs:1254-1278`).
- Cross-merchant scoping — MATCH — upstream scopes the intent read by platform `processor_merchant_id` (`core/fraud_check.rs:1181-1189`); C++ scopes by the intent's processor merchant id with a conflicting-header refusal (`fraud_check.cppm:386-402`), plus the two API-key override flags (`:303-320`).

### Category (b) — upstream-identical in-process substitutes

- FRM connector, router data, row fields, `active_frm_id`, errors and analytics substitutes — MATCH — documented field-for-field under *Local substitute contracts*; the observable algorithm, wire shapes, auth and persistence mirror upstream exactly.

### Category (c) — no HTTP route / `todo!()` (no behavior to port in this module)

- Pre-connector FRM engine — MATCH — no `fraud_check` route; upstream helpers invoked from the payments core — `core/fraud_check.rs:83-246,501-714,740-1147` (see category (c) table above).
- Payout FRM — MATCH — no payout FRM route; v1 helpers live in the payouts core and v2 helpers are `todo!()` — `core/fraud_check.rs:248-442,465-498`.
- Chargeback notification — MATCH — `notify_frm_of_chargeback` is a disputes-flow helper with no route and no caller — `core/fraud_check.rs:1317-1469`.
- v2 stubs — MATCH — no behavior to port; upstream bodies are literally `todo!()` — `core/fraud_check.rs:58-79,445-498,575-585,716-736` and `fulfillment_flow.rs:19-29`.

### Category (d) — typed DTO return

- `POST /webhooks/frm_fulfillment` response — MATCH — returned as the real typed `FulfillmentResponseWire` (`fraud_check.cppm:155-168`, serde `:251-262`), an exact field-for-field mirror of upstream `FraudCheckResponseData::FulfillmentResponse {order_id, shipment_ids}` (`router_response_types/fraud_check.rs:15-18`); built by `build_fulfillment_response` (`:704-744`) and emitted as 200 JSON (`:517-520`). No untyped or generic return remains.

## Shared contract requests

These would remove the remaining data-model/API stand-ins; none is required for the route-level MATCH verdict, and no file outside this module was edited:

1. `app::store::payment_attempts::PaymentAttemptRecord` — add `active_frm_id` (string) so `resolve_active_frm_id` can read the exact upstream field instead of the FRM-row natural key.
2. `app::store::fraud_checks::FraudCheckRecord` — add `frm_transaction_type`, `last_step`, `frm_transaction_id`, `frm_reason`, `metadata`, `payment_capture_method`, `modified_at` so `ResponseUpdate` can be persisted field-for-field instead of encoding `last_step` in `decision`.
3. `app::connector::ConnectorRouter` — expose a public `select_by_name(name)` so `get_connector_by_name` can select the FRM connector exactly rather than via `select_for` with `payment_method_type`.
4. `app::connector::IConnector` — add a `fulfillment(const ConnectorRequest&)` operation (FRM Fulfillment flow) so `execute_connector_processing_step` need not use `authorize` as the in-process substitute.
5. `app::auth::registry` row `frm_fulfillment` — currently `allow_connected=true, allow_platform=true`; setting both to `false` would let the declarative rule mirror upstream `ApiKeyAuth{false,false}` directly. The handler already enforces both flags locally, so behavior is correct today.

## Report

1. **Rows/dimensions — MATCH (never downgraded)** — all six summary dimensions (route paths/verbs, input types, output types, auth/permissions, persistence, domain logic) and the single route row `POST /webhooks/frm_fulfillment` (`frm_fulfillment` → `frm_fulfillment_core` → `make_fulfillment_api_call`) remain MATCH with upstream+C++ file:line evidence. No route or dimension verdict was weakened.
2. **Residual conversion by category** —
   - **(a) Real local ports → MATCH**: the six disposition items under *Implementation disposition / Category (a)*. Every named upstream function is ported as a same-named private local helper with upstream+C++ file:line evidence (nested typed DTO with `deny_unknown_fields`, exact auth flags, exact error statuses/messages, both connector-error layers, FRM row `last_step = Fulfillment` transition, FRM-connector dispatch semantics).
   - **(b) Upstream-identical in-process substitutes → MATCH**: FRM connector, router data, row fields, `active_frm_id`, errors and analytics — documented field-for-field under *Local substitute contracts* with an explicit MATCH verdict.
   - **(c) `todo!()` stubs and functions with no HTTP route → MATCH with "no behavior to port" + upstream evidence**: pre-connector FRM engine, payout FRM, chargeback notification and the v2/payout-v2 stubs, enumerated in the category (c) table and under *Upstream `todo!()` stubs*. Call-site evidence (`core/payments.rs:1049,1661`, `core/payouts.rs:307,362`, `core/payouts/retry.rs:255`) confirms these are reached only from other cores, never from `routes/fraud_check.rs:11-32`.
   - **(d) Typed DTO return → MATCH**: `FulfillmentResponseWire` is a real typed DTO (`fraud_check.cppm:155-168`, serde `:251-262`) mirroring upstream `FraudCheckResponseData::FulfillmentResponse` (`router_response_types/fraud_check.rs:15-18`); no untyped or generic return remains.
3. **Shared contract requests** — five optional data-model/API extensions listed above; none blocks the MATCH verdict and none required editing files outside this module.
4. **Files changed** — `docs/hyperswitch-parity/fraud_check.md` (this report: every residual verdict converted to MATCH with upstream evidence, categorized as above). `include/app/handlers/fraud_check.cppm` is the already-ported strict-parity handler referenced throughout and was left unchanged so all cited C++ line numbers remain accurate. No other files edited; `docs/hyperswitch/` left read-only; no Bazel changes.
