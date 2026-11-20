# Parity: fraud_check

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/fraud_check.rs` (path/verb from `routes/app.rs`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/fraud_check.rs`
- C++: include/app/handlers/fraud_check.cppm
- Checked: 2026-11-19

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `POST /webhooks/frm_fulfillment` on both sides; install at fraud_check.cppm:245-256 |
| Input types | PARTIAL | Typed `FrmFulfillmentRequestWire` mirrors upstream required/optional fields (fraud_check.cppm:31-59,82-99) with `payment_id`/`order_id` validation (:145-152); nested `fulfillments` still normalized from generic JSON as documented substitute (:153,265-298), no `deny_unknown_fields` |
| Output types | PARTIAL | Typed `FulfillmentResponseWire` mirrors upstream `FraudCheckResponseData::FulfillmentResponse` (fraud_check.cppm:68-78,234-238), but built from request `order_id` + extracted shipment ids, not connector result |
| Auth/permissions | PARTIAL | Route auth now from `auth_rules().rule_for("frm_fulfillment")` + `authorize_route` (fraud_check.cppm:128-130); upstream platform `ApiKeyAuth` vs local app permission — different model |
| Persistence | PARTIAL | FRM row now updated via `repository().update_fraud_check` with `status` + `decision="Fulfillment"` (fraud_check.cppm:220-226) plus webhook event (:228-229,354-375); local record lacks reason/score/metadata columns (:222-223) |
| Domain logic | PARTIAL | Succeeded gate (:172-177), FRM row lookup + termination guard (:183-191,311-327) and connector failure mapping (:211-217,329-349) now ported with local substitutes; pre/post FRM engine still absent |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| `POST /webhooks/frm_fulfillment` | `frm_fulfillment` → `frm_fulfillment_core` → `make_fulfillment_api_call` | `FraudCheckRouteHandlers::handle_frm_fulfillment` (installed as `.post`) | Upstream `FrmFulfillmentRequest` typed DTO; C++ typed `FrmFulfillmentRequestWire` (:31-59) + generic nested-fulfillment normalization (:265-298) | Upstream `FraudCheckResponseData` from connector; C++ typed `FulfillmentResponseWire` built from request `order_id` + extracted shipment ids (:68-78,234-238) | Upstream `ApiKeyAuth{allow_connected_scope_operation:false, allow_platform_self_operation:false}`; C++ `auth_rules().rule_for("frm_fulfillment")` + `authorize_route` (:128-130) | Upstream DB `find_payment_intent_by_payment_id_processor_merchant_id`, `find_payment_attempt...`, `find_fraud_check_by_frm_id`, `update_fraud_check_response_with_frm_id`; C++ `find_payment_by_string`, `find_fraud_check`/`update_fraud_check`, `connector_router().select().authorize()`, `save_webhook_event`, metrics | PARTIAL | Upstream: app.rs:2344,2362-2363; fraud_check.rs:10-31; core/fraud_check.rs:1175-1305. C++: fraud_check.cppm:119-239,241-257,265-375 |

Evidence detail:
- Route registration upstream `web::scope("/webhooks")` + `web::resource("/frm_fulfillment").route(web::post()...)` — app.rs:2344,2362-2363. C++ `Router{router, "/webhooks"}.add_route("/frm_fulfillment").post(...)` — fraud_check.cppm:245-256. MATCH.
- Upstream input `FrmFulfillmentRequest` (`#[serde(deny_unknown_fields)]`): required `payment_id`, `order_id`, `fulfillments`; optional `fulfillment_status`, `tracking_company`, `tracking_numbers`, etc. — fraud_check.rs:14 (type from hyperswitch_domain_models .../fraud_check.rs:108-133). C++ now deserializes a typed `FrmFulfillmentRequestWire` (fraud_check.cppm:31-59,82-99,139) and validates `payment_id`/`order_id` (:145-152); nested `fulfillments[].shipment_id` still normalized from generic JSON (:153,265-298). PARTIAL.
- Upstream output `RouterResponse<FraudCheckResponseData>` built from connector response — core/fraud_check.rs:1179,1269-1304. C++ returns typed `FulfillmentResponseWire{order_id, shipment_ids}` (fraud_check.cppm:68-78,234-238) assembled from the request and extracted shipment ids, not the connector result. PARTIAL.
- Upstream auth `ApiKeyAuth` (no connected scope, no platform-self op) — fraud_check.rs:25-28. C++ resolves its declarative rule via `auth_rules().rule_for("frm_fulfillment")` and calls `authorize_route` — fraud_check.cppm:128-130. Different auth model; PARTIAL.

## Not ported
- FRM row lookup + termination guard: upstream resolves `active_frm_id` → `find_fraud_check_by_frm_id` and requires `PreFrm && last_step == TransactionOrRecordRefund` or `PostFrm && last_step == CheckoutOrSale` before fulfilling; otherwise `PreconditionFailed` ("Frm pre/post flow hasn't terminated yet") — core/fraud_check.rs:1207-1231. C++ has no fraud_check row or `last_step` check; only checks payment `succeeded` (fraud_check.cppm:107-110).
- Fraud-check state update: upstream `update_fraud_check_response_with_frm_id` sets `last_step = Fulfillment` and persists frm status/id/reason/score/metadata — core/fraud_check.rs:1279-1293. C++ only writes a webhook event and cache key `"Fulfillment"` string — fraud_check.cppm:128,164-187. No durable FRM row transition.
- Missing-`active_frm_id` handling: upstream returns `InvalidRequestData` "no fraud check entry found for this payment_id" — core/fraud_check.rs:1192-1194,1233. No equivalent in C++.
- Connector error mapping: upstream maps connector failure to `ExternalConnectorError` (code/message/connector/status_code/reason) — core/fraud_check.rs:1294-1303. C++ stores connector `get_status()` as a free-form status string with no error mapping — fraud_check.cppm:120-123,179.
- Connector dispatch semantics: upstream `execute_connector_processing_step` with `Fulfillment` integration and `CallConnectorAction::Trigger` resolved by `FraudCheckConnectorData::get_connector_by_name(&fraud_check.frm_name)` — core/fraud_check.rs:1254-1277. C++ uses generic `connector->authorize(connector_request)` on the merchant's selected connector, not the FRM connector persisted on the fraud_check row — fraud_check.cppm:115-123.
- Cross-merchant scoping differs: C++ adds header `x-merchant-id`/`x-org-id` mismatch → `FORBIDDEN` (fraud_check.cppm:73-105); upstream scopes strictly by platform `processor_merchant_id`, no such header check — core/fraud_check.rs:1181-1189.
- Pre-connector FRM engine (no C++ route/equivalent): `call_frm_service` + `decide_and_run_pre_frm` + `pre_payment_frm_core` / `post_payment_frm_core` / `call_frm_before_connector_call` (incl. `should_call_frm`, `FrmRoutingAlgorithm`/`frm_configs` resolution, `FrmSuggestion` decisions, `PreFrmFailureMode` Fail-Open/Fail-Closed) — core/fraud_check.rs:83-246,500-714,738-1147.
- Payout FRM: `get_frm_merchant_connector_account_and_routing_algorithm`, `get_payout_frm_applicability`, `pre_payouts_frm_core` — core/fraud_check.rs:248-442 (v1); v2 counterparts are `todo!()` — core/fraud_check.rs:465-498.
- Chargeback notification: `notify_frm_of_chargeback` (UCS-backed FRM, `FRM_CHARGEBACK_RECEIVED`) — core/fraud_check.rs:1317-1469. No C++ counterpart.
- v2 stubs remain upstream (`call_frm_service`, `should_call_frm`, `make_frm_data_and_fraud_check_operation`, payout helpers) — core/fraud_check.rs:58-79,444-463,716-736.

## Implementation disposition

Disposition of every bullet in `## Not ported`, re-checked against include/app/handlers/fraud_check.cppm. Local substitutes keep the affected Summary rows at PARTIAL.

- FRM row lookup + termination guard — SUBSTITUTED — `find_fraud_check` resolves by `(merchant_id, payment_id)` natural key instead of `active_frm_id`; `frm_flow_terminated`/`is_terminal_step` check terminal steps — fraud_check.cppm:179-191,300-327 (local payment has no attempt/`active_frm_id`, :179-182).
- Fraud-check state update — SUBSTITUTED — `set_status` + `set_decision("Fulfillment")` followed by `repository().update_fraud_check` gives a durable FRM row transition — fraud_check.cppm:220-226; reason/score/metadata columns absent locally (:222-223).
- Missing-`active_frm_id` handling — IMPLEMENTED — absent FRM row returns `INVALID_REQUEST` "no fraud check entry found for this payment_id" — fraud_check.cppm:183-186.
- Connector error mapping — SUBSTITUTED — `connector_call_failed` + `connector_error_message` emit a deterministic `ExternalConnectorError` string with code/message/connector/status_code/reason instead of a typed connector error — fraud_check.cppm:211-217,329-349.
- Connector dispatch semantics — SUBSTITUTED — `connector_router().select(merchant_id)` + `authorize(connector_request)` with the persisted FRM name carried in `payment_method_type`; no public select-by-name — fraud_check.cppm:193-210.
- Cross-merchant scoping differs — SUBSTITUTED — header `x-merchant-id` mismatch → `FORBIDDEN` as documented deterministic stand-in for platform `processor_merchant_id` scoping — fraud_check.cppm:155-170.
- Pre-connector FRM engine (no C++ route/equivalent) — DEFERRED — no pre/post FRM route or handler; only `/frm_fulfillment` is installed — fraud_check.cppm:241-257; upstream core/fraud_check.rs:83-246,500-714,738-1147.
- Payout FRM — DEFERRED — no payout FRM route; upstream v2 payout helpers are `todo!()` — core/fraud_check.rs:248-442,465-498.
- Chargeback notification — DEFERRED — no `notify_frm_of_chargeback` counterpart; requires UCS-backed FRM and `FRM_CHARGEBACK_RECEIVED` notification service — core/fraud_check.rs:1317-1469.
- v2 stubs remain upstream — DEFERRED — upstream `todo!()` functions have no concrete behavior to port — core/fraud_check.rs:58-79,444-463,716-736.
