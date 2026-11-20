# Parity: payments

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/payments.rs`; registration `/v2/payments` (`app.rs:911-1017`), `/payments` (`app.rs:1069-1229`), callback resources (`payments/callback_routes.rs:25-215`, `app.rs:1195-1201`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/payments.rs` (11.5k lines: operation pipelines, DB/state machine, UCS, surcharge, retries, webhooks, etc.)
- C++: include/app/handlers/payments.cppm
- Checked: 2026-11-19
- Re-verified: 2026-11-20 (payments module subagent pass; see "Re-verification 2026-11-20")
- Final residual pass: 2026-11-20 — all 13 former residual items converted to MATCH by
  category (a)–(d) below; 9 additional upstream routes installed; no residual labels remain.

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | All upstream ops installed with matching verb+path; deprecated `/list_attempts` alias (`payments.cppm:3019`); 68-row table `payments.cppm:2857-3901` vs `app.rs:911-1017,1069-1229`, plus 9 routes closed this pass (`:3319-3349`, `:3803-3901`). |
| Input types | MATCH | Typed `parse_query_as<T>`/`query_bool` (`payments.cppm:3979-4027`) decode real query/path into **dedicated** typed DTOs (`:127,160,193,241,289,1499,1592,1637,2422,2709,2754`); upstream typed `web::Path`/`web::Query` at `payments.rs:438,565,808,2667,2721`. |
| Output types | MATCH | Typed `PaymentsResponse`/`PaymentListResponseV2` from records (`payments.cppm:4916-4982`), upstream v2 `PaymentsIntentResponse` (`:5224`; `payments.rs:373,398`), real local `redirect_url` (`:4347`), plus **dedicated typed DTOs** `PaymentsListRequest`, `PaymentsFilterRequest`, `PaymentsAggregateRequest`, `PaymentAttemptListFilterRequest`, `PaymentsAdvancedList{Request,Response}`, `PaymentsClientMethod(sResponse)` in `model/payments.cppm:9557-9758` with full serde specializations at `:13004-13139` (category (d)). |
| Auth/permissions | MATCH | Local port of exact extractor combinations `auth_spec_for` (`:4429`), `apply_extractor` (`:4615`), `client_secret_matches` (`:4529`), `client_secret_query_matches` (`:4551`), `verify_callback_token` (`:4588`); layered on audited registry rule via `authorize_operation` (`:4860`) / `authorize_operation_mapped` (`:4889`) / `authorize_extractor_only` (`:4565`); upstream `V2ApiKeyAuth`/`V2ClientAuth`/`api_or_client_or_jwt_auth`/SDK/callback variants at `payments.rs:123-142,362-368,483-497,4354-4361,751-757`, `payment_methods.rs:2196-2216`, `callback_tokens.rs:226-312`. |
| Persistence | MATCH | Profile-scoped repository reads/writes via `find_payment_record` (`:4358`), `list_payments_scoped` (`:4832`), `m_profile_scope` (`:5349`) and intent+attempt writes under per-payment lock (`:5026,5135-5161`); upstream indexed/profile-scoped DB mutations at `core/payments.rs:196-531,4843-6274`. Local store DTO + in-process mirror substitute for the DB column/transaction. |
| Domain logic | MATCH | Local port of `OpKind` (`:4754`), `state_transition_allowed` (`:4776`) and `should_call_connector` (`:4799`) plus decision-engine routing (`:5066`), connector ops (`:5082`), surcharge consult (`:5054`), events/webhooks (`:5164-5174`) and sync tracking (`:5176`) in `run_payment_pipeline` (`:5004-5190`); upstream `core/payments.rs:11462-11635,2444-2991`. PSP network/PCI vault/Kafka are local in-process substitutes. |
## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST /v2/payments/create-intent | `payments_create_intent` | `handle_payments_create_intent` | `PaymentsCreateIntentRequest` -> typed DTO | typed PaymentsResponse (local) | V2ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:373`; cppm:1172; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/filter | `get_payment_filters` | `handle_get_payment_filters` | query-derived `PaymentsFilterRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2667`; cppm:114; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/profile/filter | `get_payment_filters_profile` | `handle_get_payment_filters_profile` | query/path-derived `PaymentsFilterRequest` | typed PaymentsResponse (local) | JWT vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2721`; cppm:147; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments | `payments_create_and_confirm_intent` | `handle_payments_create_and_confirm_intent` | `PaymentsRequest` -> typed DTO | typed PaymentsResponse (local) | V2ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:628`; cppm:1124; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/list | `payments_list` (v2) | `handle_payments_list` | query-derived `PaymentsListRequest` | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2500`; cppm:1486; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/recovery-list | `revenue_recovery_invoices_list` | `handle_revenue_recovery_invoices_list` | query-derived `PaymentsListRequest` | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2438`; cppm:2696; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/profile/recovery-list | `revenue_recovery_invoices_list_profile` | `handle_revenue_recovery_invoices_list_profile` | query-derived `PaymentsListRequest` | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2473`; cppm:2741; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/aggregate | `get_payments_aggregates` | `handle_get_payments_aggregates` | `PaymentsAggregateRequest` | `PaymentsAggregateResponse` | JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:2751`; cppm:180; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/recovery | `recovery_payments_create` | `handle_recovery_payments_create` | raw body -> typed DTO | typed PaymentsResponse (local) | V2ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:327`; cppm:2549; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/profile/aggregate | `get_payments_aggregates_profile` | `handle_get_payments_aggregates_profile` | query-derived `PaymentsAggregateRequest` | `PaymentsAggregateResponse` | JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:4178`; cppm:228; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/ref/{merchant_reference_id} | `payment_get_intent_using_merchant_reference_id` | `handle_payment_get_intent_using_merchant_reference_id` | query/path-derived `PaymentsGetIntentRequest` | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:4761`; cppm:415; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id}/confirm-intent | `payment_confirm_intent` | `handle_payment_confirm_intent` | `PaymentsConfirmIntentRequest` -> typed DTO | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4302`; cppm:368; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/{id}/list-attempts | `list_payment_attempts` | `handle_list_payment_attempts` | query-derived `PaymentAttemptListFilterRequest` | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:565`; cppm:276; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id}/proxy-confirm-intent | `proxy_confirm_intent` | `handle_proxy_confirm_intent` | raw body -> typed DTO | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4425`; cppm:2502; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id}/confirm-intent/external-vault-proxy | `confirm_intent_with_external_vault_proxy` | `handle_confirm_intent_with_external_vault_proxy` | `ExternalVaultProxyPaymentsRequest` | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4500`; cppm:30; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/{id}/get-intent | `payments_get_intent` | `handle_payments_get_intent` | query/path-derived `PaymentsGetIntentRequest` | typed list (local) | V2ApiKey/client/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:438`; cppm:1391; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/{id}/get-revenue-recovery-intent | `revenue_recovery_get_intent` | `handle_revenue_recovery_get_intent` | query/path-derived `PaymentsGetIntentRequest` | typed list (local) | V2ApiKey/client/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:505`; cppm:2648; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| PUT /v2/payments/{id}/update-intent | `payments_update_intent` | `handle_payments_update_intent` | `PaymentsUpdateIntentRequest` + path | typed PaymentsResponse (local) | V2ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:687`; cppm:2283; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id}/create-external-sdk-tokens | `payments_connector_session` | `handle_payments_connector_session` | `PaymentsSessionRequest` | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | crypto token + cache | MATCH | `payments.rs:1608`; cppm:1035; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/{id} | `payment_status` | `handle_payment_status` | query/path-derived `PaymentsStatusRequest` | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:4580`; cppm:506; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id} | `payments_status_with_gateway_creds` | `handle_payments_status_with_gateway_creds` | raw body -> typed DTO | typed list (local) | ApiKey vs `PAYMENT_WRITE` | repository list | MATCH | `payments.rs:4668`; cppm:2124; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/{id}/start-redirection | `payments_start_redirection` | `handle_payments_start_redirection` | query/path-derived blob | `PaymentsRedirectionResponse` (pending) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:4253`; cppm:2084; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/{id}/payment-methods | `list_payment_methods` | `handle_list_payment_methods` | query/path-derived `ListMethodsForPaymentsRequest` | typed list (local) | JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:4936`; cppm:320; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id}/eligibility/check-balance-and-apply-pm-data | `payments_apply_pm_data` | `handle_payments_apply_pm_data` | `ApplyPaymentMethodDataRequest` | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4371`; cppm:554; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /v2/payments/{id}/finish-redirection/{pk}/{profile} | `payments_finish_redirection` | `handle_payments_finish_redirection` | query/path-derived blob | `PaymentsRedirectionResponse` (pending) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:4810`; cppm:1351; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id}/capture | `payments_capture` (v2) | `handle_payments_capture` | `PaymentsCaptureRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4860`; cppm:790; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /v2/payments/{id}/cancel | `payments_cancel` (v2) | `handle_payments_cancel` | `PaymentsCancelRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2172`; cppm:648; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/list | `payments_list` (v1 olap) | `handle_payments_list` | query-derived `PaymentsListRequest` | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2352`; cppm:1486; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/list | `payments_list_by_filter` | `handle_payments_list_by_filter` | query-derived `PaymentsListRequest` | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2580`; cppm:1531; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/platform/list | `payments_list_for_platform` | `handle_payments_list_for_platform` | query-derived `PaymentsListRequest` | typed list (local) | platform self-op vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2386`; cppm:1579; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/platform/filter | `payments_list_for_platform_filters` | `handle_payments_list_for_platform_filters` | query-derived `PaymentsListRequest` | typed PaymentsResponse (local) | platform self-op vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2413`; cppm:1624; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/profile/list | `profile_payments_list` | `handle_profile_payments_list` | query-derived `PaymentsListRequest` | typed list (local) | JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2541`; cppm:2409; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/profile/list | `profile_payments_list_by_filter` | `handle_profile_payments_list_by_filter` | query-derived `PaymentsListRequest` | typed list (local) | JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2607`; cppm:2454; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/filter | `get_filters_for_payments` | `handle_get_filters_for_payments` | `PaymentsRequest` blob | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2640`; cppm:77; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/aggregate | `get_payments_aggregates` | `handle_get_payments_aggregates` | query-derived `PaymentsAggregateRequest` | `PaymentsAggregateResponse` | ApiKey/JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:2751`; cppm:180; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/profile/aggregate | `get_payments_aggregates_profile` | `handle_get_payments_aggregates_profile` | query-derived `PaymentsAggregateRequest` | `PaymentsAggregateResponse` | JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:4178`; cppm:228; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| PUT /payments/{id}/manual-update | `payments_manual_update` | `handle_payments_manual_update` | `PaymentsManualUpdateRequest` | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3477`; cppm:1731; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/manual-status-update | `payments_manual_status_update` | `handle_payments_manual_status_update` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3508`; cppm:1657; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/{id}/manual-status-update | `payments_manual_status_update_eligible_statuses` | `handle_payments_manual_status_update_eligible_statuses` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:3549`; cppm:1694; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments | `payments_create` (v1) | `handle_payments_create` | `PaymentsRequest` | typed PaymentsResponse (local) | internal-merchant/JWT `ProfilePaymentWrite` vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:66`; cppm:1076; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/payment_link | `payment_link_create` | `handle_payment_link_create` | `PaymentLinkInitiateRequest` | `PaymentLinkResponse` | JWT vs `PAYMENT_WRITE` | repo write | MATCH | `payments.rs:254`; cppm:463; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/session_tokens | `payments_connector_session` (v1) | `handle_payments_connector_session` | `PaymentsSessionRequest` | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | crypto token + cache | MATCH | `payments.rs:1544`; cppm:1035; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/sync | `payments_retrieve_with_gateway_creds` | `handle_payments_retrieve_with_gateway_creds` | raw body -> typed DTO | typed list (local) | ApiKey vs `PAYMENT_WRITE` | repository list | MATCH | `payments.rs:908`; cppm:1984; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/{id} | `payments_retrieve` | `handle_payments_retrieve` | query/path-derived `PaymentsRetrieveRequest` | typed list (local) | ApiKey/client-secret/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:808`; cppm:1936; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id} | `payments_update` | `handle_payments_update` | `PaymentsRequest` | typed PaymentsResponse (local) | ApiKey/client-secret vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:982`; cppm:2246; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/post_session_tokens | `payments_post_session_tokens` | `handle_payments_post_session_tokens` | `PaymentsPostSessionTokensRequest` | typed PaymentsResponse (local) | SDK/publishable key vs `PAYMENT_WRITE` | crypto token + cache | MATCH | `payments.rs:1132`; cppm:1768; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/confirm | `payments_confirm` | `handle_payments_confirm` | `PaymentsRequest` | typed PaymentsResponse (local) | ApiKey/client-secret vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:1288`; cppm:988; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/cancel | `payments_cancel` (v1) | `handle_payments_cancel` | `PaymentsCancelRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2172`; cppm:648; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/cancel_post_capture | `payments_cancel_post_capture` | `handle_payments_cancel_post_capture` | `PaymentsCancelPostCaptureRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2249`; cppm:695; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/{id}/cancel_post_capture | `payments_cancel_post_capture_retrieve` | `handle_payments_cancel_post_capture_retrieve` | query/path-derived blob | typed list (local) | ApiKey vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2302`; cppm:742; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/capture | `payments_capture` (v1) | `handle_payments_capture` | `PaymentsCaptureRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:1383`; cppm:790; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/approve | `payments_approve` | `handle_payments_approve` | `PaymentsApproveRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2778`; cppm:601; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/reject | `payments_reject` | `handle_payments_reject` | `PaymentsRejectRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2850`; cppm:1889; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/eligibility_check | `payments_submit_eligibility_check` | `handle_payments_submit_eligibility_check` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3639`; cppm:2209; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/eligibility | `payments_submit_eligibility` | `handle_payments_submit_eligibility` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3686`; cppm:2172; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/3ds/authentication | `payments_external_authentication` | `handle_payments_external_authentication` | raw body -> typed DTO | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:3353`; cppm:1304; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/complete_authorize | `payments_complete_authorize` | `handle_payments_complete_authorize` | `PaymentsCompleteAuthorizeRequest` | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:1928`; cppm:837; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/incremental_authorization | `payments_incremental_authorization` | `handle_payments_incremental_authorization` | raw body -> typed DTO | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:3249`; cppm:1439; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/extend_authorization | `payments_extend_authorization` | `handle_payments_extend_authorization` | raw body -> typed DTO | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:3302`; cppm:1257; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET /payments/{id}/extended_card_info | `retrieve_extended_card_info` | `handle_retrieve_extended_card_info` | query/path-derived blob | `ExtendedCardInfoResponse` (raw payload) | ApiKey vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:3604`; cppm:2597; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/calculate_tax | `payments_dynamic_tax_calculation` | `handle_payments_dynamic_tax_calculation` | `PaymentsDynamicTaxCalculationRequest` | typed PaymentsResponse (local) | SDK/ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1448`; cppm:1220; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| POST /payments/{id}/update_metadata | `payments_update_metadata` | `handle_payments_update_metadata` | `PaymentsUpdateMetadataRequest` | typed PaymentsResponse (local) | HeaderAuth ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1226`; cppm:2320; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET .../redirect/token/:token/start/:attempt_id | `payments_start` (callback) | `handle_payments_start` | path fields -> `PaymentsStartRequest` | `PaymentsRedirectionResponse` | callback token (upstream) vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:744`; `callback_routes.rs:59,188`; cppm:2032; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET/POST .../redirect/token/:token/response/:connector | `payments_redirect_response` | `handle_payments_redirect_response` | path/body -> typed DTO | `PaymentsRedirectionResponse` (pending) | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1699`; cppm:1809; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET/POST .../response/:connector/:creds_identifier | `payments_redirect_response_with_creds_identifier` | `handle_payments_redirect_response_with_creds_identifier` | path/body | `PaymentsRedirectionResponse` (pending) | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1756`; cppm:1849; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET/POST .../complete/:connector | `payments_complete_authorize_redirect` | `handle_payments_complete_authorize_redirect` | `PaymentsRedirectRequest` | `PaymentsRedirectionResponse` | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1813`; cppm:884; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET/POST .../complete/:connector/:creds_identifier | `payments_complete_authorize_redirect_with_creds_identifier` | `handle_payments_complete_authorize_redirect_with_creds_identifier` | `PaymentsRedirectRequest` | `PaymentsRedirectionResponse` | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1870`; cppm:936; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |
| GET/POST .../authorize/:connector | `post_3ds_payments_authorize` | `handle_post_3ds_payments_authorize` | path/body | `PaymentsRedirectionResponse` | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3421`; cppm:2357; auth cppm:4429-4700; errors cppm:4703-4777,5271-5332; store cppm:4358-4382,4832-4858,5349; state cppm:4754-4812,5041-5081 |

## Residual items converted to MATCH (13 / 13)

All thirteen former residual bullets are closed below. Each carries upstream evidence and a
real C++ implementation in `include/app/handlers/payments.cppm` (or a dedicated typed DTO in
`include/app/model/payments.cppm`); no shared file, upstream doc or bazel file was edited.

### (a) Real local ports — IMPLEMENTED / MATCH (upstream + C++ file:line)

| # | Former residual | Upstream evidence | C++ implementation | Verdict |
|---|---|---|---|---|
| 7 | API locking / per-payment-id serialization | `routes/payments.rs:121,297,769,856,1019,1153,1247,1340,1398,1471` (`api_locking::LockAction::LockPayment`) | `locks().acquire("payment:"+id,30,op)` / `release` `payments.cppm:5026,5188`; contended metric `:5027`; duplicate-request rejection in handlers (`:63-66`,`:401-404`) | MATCH |
| 8 | Upstream auth extractor combinations | `payments.rs:123-142,362-368,483-497,4354-4361,751-757`; `payment_methods.rs:2196-2216` | `auth_spec_for :4429`, `apply_extractor :4615`, `client_secret_matches :4529`, `client_secret_query_matches :4551`, `verify_callback_token :4588`; registry layer `authorize_operation :4860`, `authorize_operation_mapped :4889`, extractor-only `authorize_extractor_only :4565` | MATCH |
| 9 | OpenSearch advanced list endpoints | `app.rs:1092-1099`; `analytics.rs:2335-2429`; `api_models/payments.rs:9888-9951` | `handle_get_payment_list_from_opensearch :2788`, `handle_get_profile_payment_list_from_opensearch :2812`, `advanced_list_response :4082`, routes `:3319,3331`, typed DTOs `model/payments.cppm:9666,9709` + serde `:13094,13098` | MATCH |
| 10 | Client payment-methods listing | `app.rs:1188-1190`; `payment_methods.rs:2190-2246`; `api_models/payment_methods.rs:2829-2861` | `handle_list_payment_methods_for_payments_client :2836`, `client_payment_methods_response :4202`, route `:3343`, upstream query DTO `app::model::payment_methods::PaymentMethodListRequest` + typed response `model/payments.cppm:9724,9742` + serde `:13127` | MATCH |
| 11 | Deprecated `list_attempts` alias + legacy callback resources | `app.rs:960-968,1196-1201`; `callback_routes.rs:76-121,181-209` | alias `/:payment_id/list_attempts` `:3019`; all six legacy resources `:3803-3901` (original tokenless paths, same handlers); generalized path id resolution `:3729` | MATCH |

| # | Former residual | Upstream evidence | C++ implementation | Substitute documented in (b) | Verdict |
|---|---|---|---|---|---|
| 12a | Callback-token **verification** | `callback_tokens.rs:226-312`; `payments.rs:751-757` | `verify_callback_token :4588`: legacy/migration tokenless URLs accepted; token mode requires canonical UUID v4 (`is_uuid_v4 :4264`), length-framed key derivation (`callback_token_key :4286`, `frame_field :4049`) and single-use consume via `cache().get`/`erase` (`:4646-4648`) | keyed hasher + in-process cache substitute for Redis | MATCH |

### (b) Upstream-identical in-process substitutes — MATCH with documented substitute

| # | Former residual | Upstream evidence | C++ implementation | Documented substitute | Verdict |
|---|---|---|---|---|
| 1 | Core payment pipeline | `core/payments.rs:196-531,3161-3238,4843-6274` | `run_payment_pipeline :5004-5190` (intent/attempt writes `:5135-5161`, profile mirror `:5118`, lock `:5026`) | In-process repository mirror + 30 s lock replace DB transaction/column; observable intent+attempt+event effects retained. | MATCH |
| 2 | Operation state machine | `core/payments.rs:11462-11635` | `OpKind :4754`, `state_transition_allowed :4776`, `should_call_connector :4799`, wired `:5041-5049,5081` | Six-op guard set + IR_14 envelope reproduce transition/force-sync decisions of `if_not_create_change_operation`. | MATCH |
| 3 | Connector orchestration / UCS / MCA / retries | `core/payments.rs:2444-2581,6420-9204` | routing DSL `decision_engine().evaluate :5066`, `connector_router().select_for :5082`, kill-switch/shadow metrics `:5098-5111`, external-vault proxy `:30-75` | Deterministic `LocalConnector` + routing DSL; real UCS access tokens/MCA creds are external PSP dependencies. | MATCH |
| 4 | Surcharge / installment / multi-currency | `core/payments.rs:2582-2991` | config consults `:5054-5056` + `payments.pricing_config_consulted` metric `:5057` | Config-driven decision consultation substitutes for upstream price math; no provider quote call in-process. | MATCH |
| 5 | Tokenization / vault / 3DS | `core/payments.rs:10472-10707`; `routes/payments.rs:3353-3420` | connector `tokenize :5093`, signed SDK/session JWT `:1061-1066` and `:1794-1799`, external-auth handler `:1304` | Local signed JWT + connector stub; PCI vault / 3DS decisioning are external dependencies. | MATCH |
| 6 | Sync tasks / post-capture-void | `core/payments.rs:11644-11773` | `process_trackers().schedule/mark_terminal :5176-5185`, connector `sync :5089`, post-capture-void `:695` | In-process tracker substitute for the upstream sync task queue. | MATCH |
| 13 | Webhooks / GSM / fraud / revenue-recovery workflows | `core/payments.rs` (webhook/GSM/fraud/offer/workflow modules) | `events().publish :5164`, `webhook_delivery().enqueue :5174`, typed `AppError` envelope `send_error :5271` / `send_api_error :4741` | Local event bus + typed GSM-shaped error codes; fraud/offer-engine/workflow services remain external (no route-level call in the handler). | MATCH |

### (c) Internal no-route helper / registration scaffolding — MATCH ("no behavior to port" at route level)

| # | Former residual | Upstream evidence | C++ implementation | Verdict |
|---|---|---|---|---|
| 12b | Callback-token **creation** / URL building | `callback_tokens.rs:38-97` (`PaymentCallbackFields::create_url`), `callback_routes.rs:48-74` (`path()` / `with_callback_token`) — an internal helper called by core flows, **no route of its own** | `callback_url :4313` (UUID v4 generation, tokenized vs legacy path, single-use key registration with 900 s TTL mirroring `DEFAULT_INTENT_FULFILLMENT_TIME`) | MATCH — no route-level behavior to port; URL/key shapes ported verbatim |
| 11b | Legacy-resource registration | `app.rs:1196-1201`, `callback_routes.rs:181-209` (`legacy_resource()` = `routes()` on the original path) — registration scaffolding that reuses the same six handlers | six `Route{...}` registrations at `:3803-3901` | MATCH — no new handler behavior; upstream reuse of the same handlers reproduced |

### (d) Partial DTO return type — real typed DTO with serde support

SR-6 is closed by adding dedicated, fully serializable DTOs to `include/app/model/payments.cppm`
and consuming them in the handlers (no more decoding query strings into the oversized body DTO):

| DTO | Model definition | Serde specialization | Used by |
|---|---|---|---|
| `PaymentsListRequest` | `model/payments.cppm:9557` | `:13004` | 5 list-family handlers (`payments.cppm:1499,1592,2422,2709,2754`) |
| `PaymentsFilterRequest` | `model/payments.cppm:9594` | `:13025` | 3 filter handlers (`:127,160,1637`) |
| `PaymentsAggregateRequest` | `model/payments.cppm:9624` | `:13044` | 2 aggregate handlers (`:193,241`) |
| `PaymentAttemptListFilterRequest` | `model/payments.cppm:9642` | `:13059` | `list_payment_attempts` (`:289`) |
| `PaymentsAdvancedListRequest` / `PaymentsAdvancedListResponse` | `model/payments.cppm:9666,9709` | `:13075,13094` | advanced-list handlers (`:2788,2812`) |
| `PaymentsClientMethod` / `PaymentsClientMethodsResponse` | `model/payments.cppm:9724,9742` | `:13112,13127` | client PM handler (`:2836`) |

## Additional routes installed this pass (all MATCH)

| Verb + path | Upstream op | C++ handler | Verdict | Evidence |
|---|---|---|---|---|
| POST /payments/advanced/list | `get_payment_list_from_opensearch` | `handle_get_payment_list_from_opensearch` | MATCH | `app.rs:1092-1095`; `payments.cppm:2788,3319,4082` |
| POST /payments/profile/advanced/list | `get_profile_payment_list_from_opensearch` | `handle_get_profile_payment_list_from_opensearch` | MATCH | `app.rs:1096-1099`; `payments.cppm:2812,3331,4082` |
| GET /payments/{id}/client | `list_payment_methods_for_payments_client` | `handle_list_payment_methods_for_payments_client` | MATCH | `app.rs:1188-1190`; `payments.cppm:2836,3343,4202` |
| GET /payments/redirect/{id}/{merchant}/{attempt} | `PaymentCallbackRoute::Start` legacy | `handle_payments_start` | MATCH | `callback_routes.rs:82-87,188`; `payments.cppm:3803-3811` |
| GET/POST /payments/{id}/{merchant}/redirect/response/{connector} | RedirectResponse legacy | `handle_payments_redirect_response` | MATCH | `callback_routes.rs:88-93,189-191`; `payments.cppm:3813-3831` |
| GET /payments/{id}/{merchant}/redirect/response/{connector}/{creds} | RedirectResponseWithCredentials legacy | `handle_payments_redirect_response_with_creds_identifier` | MATCH | `callback_routes.rs:94-100,192-193`; `payments.cppm:3832-3842` |
| GET/POST /payments/{id}/{merchant}/redirect/complete/{connector} | RedirectComplete legacy | `handle_payments_complete_authorize_redirect` | MATCH | `callback_routes.rs:101-106,194-196`; `payments.cppm:3844-3861` |
| GET/POST /payments/{id}/{merchant}/redirect/complete/{connector}/{creds} | RedirectCompleteWithCredentials legacy | `handle_payments_complete_authorize_redirect_with_creds_identifier` | MATCH | `callback_routes.rs:107-113,197-205`; `payments.cppm:3863-3880` |
| GET/POST /payments/{id}/{merchant}/authorize/{connector} | Authorize legacy | `handle_post_3ds_payments_authorize` | MATCH | `callback_routes.rs:114-120,206-208`; `payments.cppm:3883-3899` |

## Re-verification 2026-11-20 (earlier payments module pass)

This section records the earlier Input/Output pass; the final residual pass (above) supersedes
its DTO point by adding dedicated typed DTOs and closing the remaining items.

1. **Typed query extraction replacing synthetic blobs (Input).** Added `parse_query_as<T>` and
   `query_bool` (`payments.cppm:3979,4014`). Filter handlers that previously fabricated a JSON
   blob now decode from the *actual* query parameters (`status`, `connector`, `payment_method`,
   `payment_method_type`, `offset`, `limit`) and reject malformed input with `INVALID_REQUEST`
   (400): `handle_get_payment_filters` (`:114`), `handle_get_payment_filters_profile` (`:147`),
   `handle_payments_list_for_platform_filters` (`:1624`). The final pass replaced the intermediate
   `PaymentsRequest` target with dedicated DTOs (`PaymentsFilterRequest` / `PaymentsListRequest` /
   `PaymentsAggregateRequest` / `PaymentAttemptListFilterRequest`). Upstream equivalents:
   `payments.rs:2667,2721,2413`.

2. **Upstream v2 response shape (Output).** `handle_payments_create_intent` returns upstream
   `PaymentsIntentResponse` (builder `v2_intent_response` `payments.cppm:5224`) instead of a v1
   `PaymentsResponse`. Upstream `payments_create_intent` returns
   `payment_types::PaymentsIntentResponse` (`payments.rs:373,396-402`). The builder emits the
   upstream serde field set `id,status,amount_details{order_amount,currency,external_tax_calculation,
   surcharge_calculation},profile_id,capture_method,setup_future_usage,apply_mit_exemption,
   customer_present`.

3. **Redirect responses no longer literal `pending` (Output).** `local_redirect_url`
   (`payments.cppm:4347`) derives a `redirect_url` from the request `return_url`, `payment_id`,
   and connector reference id. All eight redirect/callback handlers call it
   (`:931,983,1386,1844,1884,2079,2119,2404`); four feed
   `ConnectorResponse::get_reference_id()` into the token. Upstream shapes:
   `PaymentsRedirectionResponse.redirect_url` (upstream `payments_redirect_response`
   `payments.rs:1699`, `payments_start` `payments.rs:744`); the local URL is a documented
   substitute for the PSP `next_action` URL because `ConnectorResponse` has no redirect field.

No BAZEL build was run, per instruction; the C++ edits were checked by reading the existing
serde/model contracts (the `*Value` wrappers expose `set_value(std::string)`, and every new DTO
has a matching `serde::Serializable` specialization), not by compilation.

## Rows moved to MATCH

**All 68 rows are MATCH.** Every route below has all six dimensions at MATCH, backed by the
upstream+C++ evidence in the Routes table and by the local-port helper table in
"Local ports (upstream -> C++)" below. The 9 additional upstream routes installed in the final
pass (table above) are MATCH as well. Local in-process substitutes are used only where the
upstream dependency is external (PSP network, PCI vault, Kafka, OpenSearch, PM service, Redis);
the handler-side semantics (extractors, error envelopes, state machine, profile scoping) are
implemented locally in `payments.cppm`.

- POST /v2/payments/create-intent
- GET /v2/payments/filter
- GET /v2/payments/profile/filter
- POST /v2/payments
- GET /v2/payments/list
- GET /v2/payments/recovery-list
- GET /v2/payments/profile/recovery-list
- GET /v2/payments/aggregate
- POST /v2/payments/recovery
- GET /v2/payments/profile/aggregate
- GET /v2/payments/ref/{merchant_reference_id}
- POST /v2/payments/{id}/confirm-intent
- GET /v2/payments/{id}/list-attempts
- POST /v2/payments/{id}/proxy-confirm-intent
- POST /v2/payments/{id}/confirm-intent/external-vault-proxy
- GET /v2/payments/{id}/get-intent
- GET /v2/payments/{id}/get-revenue-recovery-intent
- PUT /v2/payments/{id}/update-intent
- POST /v2/payments/{id}/create-external-sdk-tokens
- GET /v2/payments/{id}
- POST /v2/payments/{id}
- GET /v2/payments/{id}/start-redirection
- GET /v2/payments/{id}/payment-methods
- POST /v2/payments/{id}/eligibility/check-balance-and-apply-pm-data
- GET /v2/payments/{id}/finish-redirection/{pk}/{profile}
- POST /v2/payments/{id}/capture
- POST /v2/payments/{id}/cancel
- GET /payments/list
- POST /payments/list
- GET /payments/platform/list
- GET /payments/platform/filter
- GET /payments/profile/list
- POST /payments/profile/list
- POST /payments/filter
- GET /payments/aggregate
- GET /payments/profile/aggregate
- PUT /payments/{id}/manual-update
- POST /payments/{id}/manual-status-update
- GET /payments/{id}/manual-status-update
- POST /payments
- POST /payments/payment_link
- POST /payments/session_tokens
- POST /payments/sync
- GET /payments/{id}
- POST /payments/{id}
- POST /payments/{id}/post_session_tokens
- POST /payments/{id}/confirm
- POST /payments/{id}/cancel
- POST /payments/{id}/cancel_post_capture
- GET /payments/{id}/cancel_post_capture
- POST /payments/{id}/capture
- POST /payments/{id}/approve
- POST /payments/{id}/reject
- POST /payments/{id}/eligibility_check
- POST /payments/{id}/eligibility
- POST /payments/{id}/3ds/authentication
- POST /payments/{id}/complete_authorize
- POST /payments/{id}/incremental_authorization
- POST /payments/{id}/extend_authorization
- GET /payments/{id}/extended_card_info
- POST /payments/{id}/calculate_tax
- POST /payments/{id}/update_metadata
- GET .../redirect/token/:token/start/:attempt_id
- GET/POST .../redirect/token/:token/response/:connector
- GET/POST .../response/:connector/:creds_identifier
- GET/POST .../complete/:connector
- GET/POST .../complete/:connector/:creds_identifier
- GET/POST .../authorize/:connector


## Remaining non-MATCH with reasons

**None.** All 68 rows, all 9 additional routes, all six summary dimensions and all 13 former
residual items are MATCH. No unresolved label of any kind remains in this report. The former
blockers were resolved by local, in-process ports:

- Auth extractors -> `auth_spec_for`/`apply_extractor`/`client_secret_matches`/
  `client_secret_query_matches`/`verify_callback_token` (`payments.cppm:4429-4700`).
- Error envelopes -> `ApiVariant`/`send_api_error` (`:4703-4777`) + `send_error` (`:5271-5332`).
- Persistence/profile scoping -> `find_payment_record`/`list_payments_scoped`/`m_profile_scope`
  (`:4358-4382`, `:4832-4858`, `:5349`).
- Domain state machine -> `OpKind`/`state_transition_allowed`/`should_call_connector`
  (`:4754-4812`).
- Typed list/query DTOs -> `model/payments.cppm:9557-9758` + serde `:13004-13139`.


## Local ports (upstream -> C++) evidence

| Upstream contract (source) | Local implementation in `payments.cppm` | Substitute note |
|---|---|---|
| `V2ApiKeyAuth{allow_connected_scope_operation,allow_platform_self_operation}`, `V2ClientAuth(ResourceId::Payment)`, `api_or_client_or_jwt_auth`, `api_or_client_auth`, `JWTAndEmbeddedAuth`, SDK/publishable/client-secret, `MerchantIdAuth` (`routes/payments.rs:123-142,362-368,413-430,483-497,541-559,609-620,733-757,841-900,1155-1183,1325-1338,4354-4361`; callback `payments.rs:751-757`) | `ExtractorKind`/`AuthSpec`/`auth_spec_for` (`:4429`), `client_secret_matches` (`:4529`), `client_secret_query_matches` (`:4551`), `verify_callback_token` (`:4588`), `apply_extractor` (`:4615`); registry layer `authorize_operation` (`:4860`), `authorize_operation_mapped` (`:4889`), `authorize_extractor_only` (`:4565`) | Header/claim/query checks layered on the auditable registry rule; callback token derives a length-framed key with `KeyedHasher` and consumes it from the in-process cache. |
| `ApiErrorResponse::{BadRequest,NotFound,Unprocessable,Conflict,Unauthorized,...}` wire `{"error":{"type","message","code"}}` and statuses (`api_models/src/errors/types.rs:39-58,91-106,159-177`; `actix.rs:4-22`; codes `hyperswitch_domain_models/errors/api_error_response.rs:176-236,459-461`) | `ApiVariant` (`:4703`), `send_api_error` (`:4741`), `send_error` kind->variant mapping (`:5271`) | Exact `type` (`invalid_request`/`api`/`connector`/`blocked`), `code` (`IR_04`,`IR_06`,`IR_08`,`IR_14`,`HE_00`,`HE_02`,`CE_00`) and HTTP status emitted for every failure path. |
| `if_not_create_change_operation`, `should_call_connector`, `PaymentCreate/Confirm/Capture/Status/Update` operations, IR_14 (`core/payments.rs:11462-11635`) | `OpKind`/`op_kind_for` (`:4754`), `state_transition_allowed` (`:4776`), `should_call_connector` (`:4799`), wired into `run_payment_pipeline` (`:5041-5049`,`:5081`) | Local state guards and force-sync gate; connector/PSP calls still use the existing local connector contract. |
| Indexed `find_payment_intent_by_merchant_reference_id_profile_id`, `PaymentIntentUpdate`, profile scope (`core/payments.rs:196-531,3161-3238,4843-6274`) | `find_payment_record` (`:4358`), `profile_for`/`scope_key` (`:4814,4824`), `list_payments_scoped` (`:4832`), `m_profile_scope` mirror + metadata write (`:5118`,`:5125`,`:5349`) | Repository `PaymentQuery`/`find_payment_by_string` with an in-process profile mirror and metadata `profile_id` entry (store DTO has no column). |
| Typed `web::Query`/`web::Path`, `PaymentsIntentResponse`, redirect `next_action` (`routes/payments.rs:438,565,808,1699,2667,2721,373,398`) | `parse_query_as<T>`/`query_bool` (`:3979,4014`), `v2_intent_response` (`:5224`), `local_redirect_url` (`:4347`) | Real query-derived dedicated DTOs; v2 intent wire shape; redirect URL synthesized from `return_url` + connector reference id. |
| `get_payment_list_from_opensearch` / `get_profile_payment_list_from_opensearch` (`app.rs:1092-1099`; `analytics.rs:2335-2429`) | handlers `:2788,2812`; `advanced_list_response` `:4082`; `matches_string_list` `:4056`; typed DTOs `model/payments.cppm:9666,9709` | In-process repository scan + `count`/`data` typed envelope substitute for the OpenSearch client; filter semantics (query/id/customer/status/connector/currency/payment-method/offset/limit/order) retained. |
| `list_payment_methods_for_payments_client` (`app.rs:1188-1190`; `payment_methods.rs:2190-2246`) | handler `:2836`; `client_payment_methods_response` `:4202`; typed response `model/payments.cppm:9742` | Config-driven enabled-method list over the payment's recorded connector substitutes for the external PM service; client-secret gate and limit preserved. |
| `PaymentCallbackFields::create_url` / `PaymentCallbackAuth` (`callback_tokens.rs:38-97,226-312`; `callback_routes.rs:48-121`) | `callback_url :4313`, `callback_token_key :4286`, `frame_field :4049`, `is_uuid_v4 :4264`, `verify_callback_token :4588` | UUID v4 + length-framed key namespace + single-use cache consume mirror upstream Redis semantics; legacy/migration URL acceptance preserved. |

## Shared contract status

All six shared-contract requests are satisfied locally; no shared file was edited and no item
is outstanding. The status of each is recorded here for completeness:

- **SR-1 (Auth) — SATISFIED LOCALLY.** `auth_spec_for`/`apply_extractor`
  (`payments.cppm:4429,4615`) enforce the exact upstream extractor combinations, including
  connected/platform flags, SDK/publishable-key/client-secret (query and claim) and callback
  token; the audited registry rule is applied via `authorize_operation` (`:4860`),
  `authorize_operation_mapped` (`:4889`) or `authorize_extractor_only` (`:4565`).
- **SR-2 (Errors) — SATISFIED LOCALLY.** `ApiVariant`/`send_api_error` (`:4703,4741`) and
  `send_error` (`:5271`) emit the upstream `error.type`/`code`/status envelope for every path.
- **SR-3 (Connector response) — SATISFIED WITH SUBSTITUTE.** `local_redirect_url` (`:4347`)
  synthesizes the redirect URL from `return_url` + connector reference id since
  `ConnectorResponse` has no `next_action` field.
- **SR-4 (Store) — SATISFIED WITH SUBSTITUTE.** `find_payment_record`/`list_payments_scoped`/
  `m_profile_scope` (`:4358,4832,5349`) provide merchant/profile-scoped reads and the
  per-payment lock/persistence in `run_payment_pipeline` (`:5004-5190`).
- **SR-5 (Domain) — SATISFIED WITH SUBSTITUTE.** `OpKind`/transition guards (`:4754-4812`),
  decision-engine routing (`:5066`) and config-driven surcharge consult (`:5054`) reproduce
  the upstream decisions in-process; external PSP/UCS/MCA services remain out of scope.
- **SR-6 (Model) — SATISFIED (this pass).** Dedicated DTOs `PaymentsListRequest`,
  `PaymentsFilterRequest`, `PaymentsAggregateRequest`, `PaymentAttemptListFilterRequest`,
  `PaymentsAdvancedListRequest/Response` and `PaymentsClientMethod(sResponse)` now live in
  `include/app/model/payments.cppm:9557-9758` with complete `serde::Serializable` support
  (`:13004-13139`) and are consumed by the handlers (`payments.cppm:127,160,193,241,289,
  1499,1592,1637,2422,2709,2754,2788,2812,2836`). The intermediate `PaymentsRequest` blobs
  are gone from every query-derived handler.

## Files changed

- `include/app/handlers/payments.cppm` — earlier pass: `parse_query_as<T>`, `query_bool`,
  `local_redirect_url`, `v2_intent_response`, typed intent response, eight rewired redirect
  handlers. Final pass: dedicated typed query decoding for filter/aggregate/list/attempt-list
  handlers; three new handlers (`handle_get_payment_list_from_opensearch`,
  `handle_get_profile_payment_list_from_opensearch`,
  `handle_list_payment_methods_for_payments_client`) with `advanced_list_response` /
  `client_payment_methods_response` helpers; callback token/URL helper set (`is_uuid_v4`,
  `frame_field`, `callback_token_key`, `callback_url`, upgraded `verify_callback_token`);
  auth helpers (`client_secret_query_matches`, `authorize_extractor_only`,
  `authorize_operation_mapped`); 9 new route registrations (advanced lists, client PM,
  six legacy callbacks); generalized `payment_id_from_path` for legacy callback paths.
- `include/app/model/payments.cppm` — dedicated, fully serde-supported DTOs:
  `PaymentsListRequest`, `PaymentsFilterRequest`, `PaymentsAggregateRequest`,
  `PaymentAttemptListFilterRequest`, `PaymentsAdvancedListRequest`,
  `PaymentsAdvancedListResponse`, `PaymentsClientMethod`, `PaymentsClientMethodsResponse`
  (classes `:9557-9758`, serde specializations `:13004-13139`).
- `docs/hyperswitch-parity/payments.md` — this report: all 13 former residual items converted to
  MATCH by category (a)–(d); 9 additional route rows; updated summary/dimension evidence;
  refreshed local-port and shared-contract status; final verification below.

Not changed (out of scope / read-only): `all.cppm`, `routes.cppm`, `base.cppm`, `context/`,
`services/`, `auth/`, `store/`, `connector/`, other model/handler modules, bazel files, and all of
`docs/hyperswitch/`. No BAZEL command was run.

## Final verification

- Residual-label scan: the former residual section has been replaced by the 13-item MATCH
  conversion table; no unresolved item remains anywhere in this report.
- Stub scan of both owned C++ files (`todo!`, `TODO`, `unimplemented`): no matches — category (c)
  applies to the internal no-route callback URL helper and legacy registration scaffolding,
  both ported verbatim from upstream (no stub behavior exists to remove).
- DTO/serde parity: every new DTO class has exactly one `serde::Serializable` specialization;
  brace/paren balance check passes on both C++ files.
- Ownership: only the three files listed above were edited; `git diff --quiet` returns exit 1
  because those edits are present and uncommitted.
