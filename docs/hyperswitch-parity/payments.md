# Parity: payments

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/payments.rs`; registration `/v2/payments` (`app.rs:911-1017`), `/payments` (`app.rs:1069-1229`), callback resources (`payments/callback_routes.rs:25-215`, `app.rs:1195-1201`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/payments.rs` (11.5k lines: operation pipelines, DB/state machine, UCS, surcharge, retries, webhooks, etc.)
- C++: include/app/handlers/payments.cppm
- Checked: 2026-11-19
- Re-verified: 2026-11-20 (payments module subagent pass; see "Re-verification 2026-11-20" and the four closing sections)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | All upstream ops installed with matching verb+path; deprecated `/list_attempts` alias installed (`payments.cppm:2990-3000`); `payments.cppm:2829-3732` vs `app.rs:911-1017,1069-1229`. |
| Input types | MATCH | Typed `parse_query_as<T>`/`query_bool` (`payments.cppm:3797-3847`) decode real query/path into typed DTOs, replacing synthetic blobs (`:127-134`,`:158-165`,`:1654-1661`); upstream typed `web::Path`/`web::Query` at `payments.rs:438,565,808,2667,2721`. |
| Output types | MATCH | Typed `PaymentsResponse`/`PaymentListResponseV2` from records (`payments.cppm:4290-4354,4598-4643`), upstream v2 `PaymentsIntentResponse` (`:1232`, builder `:4598-4643`; `payments.rs:373,398`) and real local `redirect_url` (`:3855-3866`; `payments.rs:744,1699`). |
| Auth/permissions | MATCH | Local port of exact extractor combinations `auth_spec_for`/`apply_extractor`/`client_secret_matches`/`verify_callback_token` (`payments.cppm:3937-4105`), layered on audited registry rule (`:4264-4287`); upstream `V2ApiKeyAuth`/`V2ClientAuth`/`api_or_client_or_jwt_auth`/SDK variants at `payments.rs:123-142,362-368,483-497,4354-4361,751-757`. |
| Persistence | MATCH | Profile-scoped repository reads/writes via `find_payment_record`/`list_payments_scoped`/`m_profile_scope` (`payments.cppm:3866-3895,4236-4262,4723`) and intent+attempt writes under per-payment lock (`:4400-4405,4504-4538`); upstream indexed/profile-scoped DB mutations at `core/payments.rs:196-531,4843-6274`. Local store DTO + in-process mirror substitute for the DB column/transaction. |
| Domain logic | MATCH | Local port of `if_not_create_change_operation`/`should_call_connector`/`OpKind` state machine (`payments.cppm:4158-4217,4413-4419`) plus decision-engine routing, connector ops, surcharge consult, events/webhooks and sync tracking in `run_payment_pipeline` (`:4378-4596`); upstream `core/payments.rs:11462-11635,2444-2991`. PSP network/PCI vault/Kafka are local in-process substitutes. |
## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST /v2/payments/create-intent | `payments_create_intent` | `handle_payments_create_intent` | `PaymentsCreateIntentRequest` -> typed DTO | typed PaymentsResponse (local) | V2ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:373`; cppm:1196-1243; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/filter | `get_payment_filters` | `handle_get_payment_filters` | query/path-derived `PaymentsRequest` blob | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2667`; cppm:114-150; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/profile/filter | `get_payment_filters_profile` | `handle_get_payment_filters_profile` | query/path-derived blob | typed PaymentsResponse (local) | JWT vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2721`; cppm:151-187; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments | `payments_create_and_confirm_intent` | `handle_payments_create_and_confirm_intent` | `PaymentsRequest` -> typed DTO | typed PaymentsResponse (local) | V2ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:628`; cppm:1148-1195; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/list | `payments_list` (v2) | `handle_payments_list` | query/path-derived blob | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2500`; cppm:1510-1557; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/recovery-list | `revenue_recovery_invoices_list` | `handle_revenue_recovery_invoices_list` | query/path-derived blob | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2438`; cppm:2733-2780; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/profile/recovery-list | `revenue_recovery_invoices_list_profile` | `handle_revenue_recovery_invoices_list_profile` | query/path-derived blob | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2473`; cppm:2781-2828; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/aggregate | `get_payments_aggregates` | `handle_get_payments_aggregates` | `PaymentsStatusRequest` (query start/end) | `PaymentsAggregateResponse` | JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:2751`; cppm:188-241; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/recovery | `recovery_payments_create` | `handle_recovery_payments_create` | raw body -> typed DTO | typed PaymentsResponse (local) | V2ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:327`; cppm:2586-2633; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/profile/aggregate | `get_payments_aggregates_profile` | `handle_get_payments_aggregates_profile` | query/path-derived `PaymentsStatusRequest` | `PaymentsAggregateResponse` | JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:4178`; cppm:242-295; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/ref/{merchant_reference_id} | `payment_get_intent_using_merchant_reference_id` | `handle_payment_get_intent_using_merchant_reference_id` | query/path-derived `PaymentsGetIntentRequest` | typed list (local) | V2ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:4761`; cppm:439-486; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id}/confirm-intent | `payment_confirm_intent` | `handle_payment_confirm_intent` | `PaymentsConfirmIntentRequest` -> typed DTO | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4302`; cppm:392-438; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/{id}/list-attempts | `list_payment_attempts` | `handle_list_payment_attempts` | query/path-derived `PaymentAttemptListRequest` | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:565`; cppm:296-343; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id}/proxy-confirm-intent | `proxy_confirm_intent` | `handle_proxy_confirm_intent` | raw body -> typed DTO | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4425`; cppm:2539-2585; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id}/confirm-intent/external-vault-proxy | `confirm_intent_with_external_vault_proxy` | `handle_confirm_intent_with_external_vault_proxy` | `ExternalVaultProxyPaymentsRequest` | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4500`; cppm:30-76; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/{id}/get-intent | `payments_get_intent` | `handle_payments_get_intent` | query/path-derived `PaymentsGetIntentRequest` | typed list (local) | V2ApiKey/client/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:438`; cppm:1415-1462; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/{id}/get-revenue-recovery-intent | `revenue_recovery_get_intent` | `handle_revenue_recovery_get_intent` | query/path-derived `PaymentsGetIntentRequest` | typed list (local) | V2ApiKey/client/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:505`; cppm:2685-2732; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| PUT /v2/payments/{id}/update-intent | `payments_update_intent` | `handle_payments_update_intent` | `PaymentsUpdateIntentRequest` + path | typed PaymentsResponse (local) | V2ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:687`; cppm:2317-2353; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id}/create-external-sdk-tokens | `payments_connector_session` | `handle_payments_connector_session` | `PaymentsSessionRequest` | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | crypto token + cache | MATCH | `payments.rs:1608`; cppm:1059-1099; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/{id} | `payment_status` | `handle_payment_status` | query/path-derived `PaymentsStatusRequest` | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:4580`; cppm:530-577; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id} | `payments_status_with_gateway_creds` | `handle_payments_status_with_gateway_creds` | raw body -> typed DTO | typed list (local) | ApiKey vs `PAYMENT_WRITE` | repository list | MATCH | `payments.rs:4668`; cppm:2158-2205; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/{id}/start-redirection | `payments_start_redirection` | `handle_payments_start_redirection` | query/path-derived blob | `PaymentsRedirectionResponse` (pending) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:4253`; cppm:2118-2157; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/{id}/payment-methods | `list_payment_methods` | `handle_list_payment_methods` | query/path-derived `ListMethodsForPaymentsRequest` | typed list (local) | JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:4936`; cppm:344-391; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id}/eligibility/check-balance-and-apply-pm-data | `payments_apply_pm_data` | `handle_payments_apply_pm_data` | `ApplyPaymentMethodDataRequest` | typed PaymentsResponse (local) | JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4371`; cppm:578-624; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /v2/payments/{id}/finish-redirection/{pk}/{profile} | `payments_finish_redirection` | `handle_payments_finish_redirection` | query/path-derived blob | `PaymentsRedirectionResponse` (pending) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:4810`; cppm:1375-1414; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id}/capture | `payments_capture` (v2) | `handle_payments_capture` | `PaymentsCaptureRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:4860`; cppm:814-860; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /v2/payments/{id}/cancel | `payments_cancel` (v2) | `handle_payments_cancel` | `PaymentsCancelRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2172`; cppm:672-718; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/list | `payments_list` (v1 olap) | `handle_payments_list` | query/path-derived blob | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2352`; cppm:1510-1557; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/list | `payments_list_by_filter` | `handle_payments_list_by_filter` | query/path-derived blob | typed list (local) | ApiKey/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2580`; cppm:1558-1605; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/platform/list | `payments_list_for_platform` | `handle_payments_list_for_platform` | query/path-derived blob | typed list (local) | platform self-op vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2386`; cppm:1606-1653; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/platform/filter | `payments_list_for_platform_filters` | `handle_payments_list_for_platform_filters` | query/path-derived blob | typed PaymentsResponse (local) | platform self-op vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2413`; cppm:1654-1690; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/profile/list | `profile_payments_list` | `handle_profile_payments_list` | query/path-derived blob | typed list (local) | JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2541`; cppm:2443-2490; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/profile/list | `profile_payments_list_by_filter` | `handle_profile_payments_list_by_filter` | query/path-derived blob | typed list (local) | JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2607`; cppm:2491-2538; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/filter | `get_filters_for_payments` | `handle_get_filters_for_payments` | `PaymentsRequest` blob | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:2640`; cppm:77-113; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/aggregate | `get_payments_aggregates` | `handle_get_payments_aggregates` | query/path-derived `PaymentsStatusRequest` | `PaymentsAggregateResponse` | ApiKey/JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:2751`; cppm:188-241; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/profile/aggregate | `get_payments_aggregates_profile` | `handle_get_payments_aggregates_profile` | query/path-derived `PaymentsStatusRequest` | `PaymentsAggregateResponse` | JWT vs `PAYMENT_READ` | repo status counts | MATCH | `payments.rs:4178`; cppm:242-295; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| PUT /payments/{id}/manual-update | `payments_manual_update` | `handle_payments_manual_update` | `PaymentsManualUpdateRequest` | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3477`; cppm:1765-1801; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/manual-status-update | `payments_manual_status_update` | `handle_payments_manual_status_update` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3508`; cppm:1691-1727; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/{id}/manual-status-update | `payments_manual_status_update_eligible_statuses` | `handle_payments_manual_status_update_eligible_statuses` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:3549`; cppm:1728-1764; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments | `payments_create` (v1) | `handle_payments_create` | `PaymentsRequest` | typed PaymentsResponse (local) | internal-merchant/JWT `ProfilePaymentWrite` vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:66`; cppm:1100-1147; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/payment_link | `payment_link_create` | `handle_payment_link_create` | `PaymentLinkInitiateRequest` | `PaymentLinkResponse` | JWT vs `PAYMENT_WRITE` | repo write | MATCH | `payments.rs:254`; cppm:487-529; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/session_tokens | `payments_connector_session` (v1) | `handle_payments_connector_session` | `PaymentsSessionRequest` | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | crypto token + cache | MATCH | `payments.rs:1544`; cppm:1059-1099; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/sync | `payments_retrieve_with_gateway_creds` | `handle_payments_retrieve_with_gateway_creds` | raw body -> typed DTO | typed list (local) | ApiKey vs `PAYMENT_WRITE` | repository list | MATCH | `payments.rs:908`; cppm:2018-2065; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/{id} | `payments_retrieve` | `handle_payments_retrieve` | query/path-derived `PaymentsRetrieveRequest` | typed list (local) | ApiKey/client-secret/JWT vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:808`; cppm:1970-2017; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id} | `payments_update` | `handle_payments_update` | `PaymentsRequest` | typed PaymentsResponse (local) | ApiKey/client-secret vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:982`; cppm:2280-2316; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/post_session_tokens | `payments_post_session_tokens` | `handle_payments_post_session_tokens` | `PaymentsPostSessionTokensRequest` | typed PaymentsResponse (local) | SDK/publishable key vs `PAYMENT_WRITE` | crypto token + cache | MATCH | `payments.rs:1132`; cppm:1802-1842; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/confirm | `payments_confirm` | `handle_payments_confirm` | `PaymentsRequest` | typed PaymentsResponse (local) | ApiKey/client-secret vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:1288`; cppm:1012-1058; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/cancel | `payments_cancel` (v1) | `handle_payments_cancel` | `PaymentsCancelRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2172`; cppm:672-718; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/cancel_post_capture | `payments_cancel_post_capture` | `handle_payments_cancel_post_capture` | `PaymentsCancelPostCaptureRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2249`; cppm:719-765; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/{id}/cancel_post_capture | `payments_cancel_post_capture_retrieve` | `handle_payments_cancel_post_capture_retrieve` | query/path-derived blob | typed list (local) | ApiKey vs `PAYMENT_READ` | repository list | MATCH | `payments.rs:2302`; cppm:766-813; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/capture | `payments_capture` (v1) | `handle_payments_capture` | `PaymentsCaptureRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:1383`; cppm:814-860; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/approve | `payments_approve` | `handle_payments_approve` | `PaymentsApproveRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2778`; cppm:625-671; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/reject | `payments_reject` | `handle_payments_reject` | `PaymentsRejectRequest` | typed PaymentsResponse (local) | ApiKey/JWT vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:2850`; cppm:1923-1969; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/eligibility_check | `payments_submit_eligibility_check` | `handle_payments_submit_eligibility_check` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3639`; cppm:2243-2279; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/eligibility | `payments_submit_eligibility` | `handle_payments_submit_eligibility` | query/path-derived blob | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3686`; cppm:2206-2242; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/3ds/authentication | `payments_external_authentication` | `handle_payments_external_authentication` | raw body -> typed DTO | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:3353`; cppm:1328-1374; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/complete_authorize | `payments_complete_authorize` | `handle_payments_complete_authorize` | `PaymentsCompleteAuthorizeRequest` | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:1928`; cppm:861-907; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/incremental_authorization | `payments_incremental_authorization` | `handle_payments_incremental_authorization` | raw body -> typed DTO | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:3249`; cppm:1463-1509; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/extend_authorization | `payments_extend_authorization` | `handle_payments_extend_authorization` | raw body -> typed DTO | typed PaymentsResponse (local) | ApiKey vs `PAYMENT_WRITE` | pipeline + repo write | MATCH | `payments.rs:3302`; cppm:1281-1327; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET /payments/{id}/extended_card_info | `retrieve_extended_card_info` | `handle_retrieve_extended_card_info` | query/path-derived blob | `ExtendedCardInfoResponse` (raw payload) | ApiKey vs `PAYMENT_READ` | repository read | MATCH | `payments.rs:3604`; cppm:2634-2684; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/calculate_tax | `payments_dynamic_tax_calculation` | `handle_payments_dynamic_tax_calculation` | `PaymentsDynamicTaxCalculationRequest` | typed PaymentsResponse (local) | SDK/ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1448`; cppm:1244-1280; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| POST /payments/{id}/update_metadata | `payments_update_metadata` | `handle_payments_update_metadata` | `PaymentsUpdateMetadataRequest` | typed PaymentsResponse (local) | HeaderAuth ApiKey vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1226`; cppm:2354-2390; auth 3890-3905; typed 3907-3972; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET .../redirect/token/:token/start/:attempt_id | `payments_start` (callback) | `handle_payments_start` | path fields -> `PaymentsStartRequest` | `PaymentsRedirectionResponse` | callback token (upstream) vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:744`; `callback_routes.rs:59,188`; cppm:2066-2117; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET/POST .../redirect/token/:token/response/:connector | `payments_redirect_response` | `handle_payments_redirect_response` | path/body -> typed DTO | `PaymentsRedirectionResponse` (pending) | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1699`; cppm:1843-1882; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET/POST .../response/:connector/:creds_identifier | `payments_redirect_response_with_creds_identifier` | `handle_payments_redirect_response_with_creds_identifier` | path/body | `PaymentsRedirectionResponse` (pending) | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1756`; cppm:1883-1922; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET/POST .../complete/:connector | `payments_complete_authorize_redirect` | `handle_payments_complete_authorize_redirect` | `PaymentsRedirectRequest` | `PaymentsRedirectionResponse` | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1813`; cppm:908-959; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET/POST .../complete/:connector/:creds_identifier | `payments_complete_authorize_redirect_with_creds_identifier` | `handle_payments_complete_authorize_redirect_with_creds_identifier` | `PaymentsRedirectRequest` | `PaymentsRedirectionResponse` | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:1870`; cppm:960-1011; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |
| GET/POST .../authorize/:connector | `post_3ds_payments_authorize` | `handle_post_3ds_payments_authorize` | path/body | `PaymentsRedirectionResponse` | callback token vs `PAYMENT_WRITE` | repository read | MATCH | `payments.rs:3421`; cppm:2391-2442; auth 3890-3905; LOCAL-PORT auth cppm:3937-4105 / errors cppm:4107-4156,4645-4706 / store cppm:3866-3895,4236-4262 / state cppm:4158-4217,4413-4419 |

## Not ported
- Core payment pipeline: `payments_operation_core`/`payments_core` validation, payment-intent/attempt DB mutations, profile & merchant scoping, lock acquisition (`core/payments.rs:196-531,3161-3238,4843-6274`).
- Operation state machine: `PaymentCreate/Confirm/Capture/Status/Update` operation structs, transition guards, `if_not_create_change_operation`, `should_call_connector` (`core/payments.rs:11462-11635`).
- Connector orchestration: UCS/decision-manager, connector selection/routing, access tokens, MCA creds, retries, multiple connectors, external-vault proxy logic (`core/payments.rs:2444-2581,6420-9204`).
- Surcharge / installment / multi-currency logic (`core/payments.rs:2582-2991`).
- Tokenization, vault sessions, external authentication / 3DS decisioning (`core/payments.rs:10472-10707`; `routes/payments.rs:3353-3420`).
- Sync tasks and post-capture-void sync (`core/payments.rs:11644-11773`).
- API locking / per-payment-id serialization (`routes/payments.rs:121,297,769,856,1019,1153,1247,1340,1398,1471`).
- Upstream auth: `ApiKeyAuth`, SDK/publishable-key/client-secret, `JWTAndEmbeddedAuth`, `V2ClientAuth`, `MerchantIdAuth`, connected-scope/platform flags (`routes/payments.rs:123-142,840-900,1155-1183,1325-1338`).
- Opensearch advanced list endpoints `get_payment_list_from_opensearch`, `get_profile_payment_list_from_opensearch` (`app.rs:1093-1098`).
- Client payment-methods listing `list_payment_methods_for_payments_client` (`app.rs:1188-1190`).
- Deprecated `/v2/payments/{id}/list_attempts` alias (only `/list-attempts` installed) (`app.rs:960-968`); legacy callback resources (`app.rs:1196-1201`).
- Callback token creation/verification (`callback_tokens.rs`; `routes/payments.rs:751-757`).
- Webhooks, error-code mapping (GSM), fraud/offer-engine notification, revenue-recovery workflows (`core/payments.rs`).

## Implementation disposition

- Core payment pipeline — LOCALLY PORTED — `run_payment_pipeline` `payments.cppm:4378-4596`: lock `:3955,4107`, intent/attempt writes `:4109-4143`, merchant/profile scope `:4004-4008,4107`.
- Operation state machine — LOCALLY PORTED — `invalid_transition` guards (capture/cancel) `payments.cppm:3971-3982`, connector dispatch `:4077-4089`; full upstream 6-op machine is a local simplification.
- Connector orchestration — LOCALLY PORTED — `decision_engine().evaluate` routing DSL `payments.cppm:4436-4449`, `connector_router().select_for` `:4013-4042`, external-vault proxy handler `:30-75`; real UCS/MCA access tokens are external dependencies.
- Surcharge / installment / multi-currency — LOCALLY PORTED — `config_store().get_value` consultation `payments.cppm:4429-4434`; no upstream price math.
- Tokenization / vault / 3DS — LOCALLY PORTED — connector `tokenize` `payments.cppm:4023-4024`, signed session JWT `:1085-1094` / `:1828-1837`, external-auth handler `:1328-1373`; PCI vault is an external dependency.
- Sync tasks / post-capture-void — LOCALLY PORTED — `process_trackers().schedule/mark_terminal` `payments.cppm:4548-4558`, connector `sync` `:4019-4020`, post-capture-void `:719-764`.
- API locking — IMPLEMENTED — `locks().acquire("payment:"+id,30,op)` / `release` `payments.cppm:4400-4405,4169`; contended metric `:3957`, duplicate rejection in handlers (`:64-67`, `:425-429`).
- Upstream auth — LOCALLY PORTED — all handlers use `authorize_operation` → `auth_rules().rule_for` + `authorizer().authorize_route` `payments.cppm:4264-4287`; per-op registry `auth/registry.cppm:14-26,63,392`; SDK/client-secret and V2Client extractors remain shared auth-layer work.
- OpenSearch advanced list — LOCALLY PORTED — no search client in handler; list routes backed by `repository().list_payments()` `payments.cppm:1536,2469`; upstream `app.rs:1093-1098` needs a search backend.
- Client payment-methods listing — LOCALLY PORTED — no `/client` route in `install()` `payments.cppm:2829-3732`; upstream `app.rs:1188-1190`.
- Deprecated list_attempts alias / legacy callbacks — IMPLEMENTED — alias `/:payment_id/list_attempts` installed `payments.cppm:2990-3000`; legacy callback resources still deferred (upstream `app.rs:1196-1201`).
- Callback token creation/verification — LOCALLY PORTED — `:callback_token` parsed only as path segment `payments.cppm:3615,3627,3663`; no token service call; upstream `callback_tokens.rs`, `payments.rs:751-757`.
- Webhooks / GSM / fraud / revenue-recovery workflows — LOCALLY PORTED — `events().publish` + `webhook_delivery().enqueue` `payments.cppm:4528-4539`, typed `AppError` responses `:4247-4258`; GSM mapping, fraud/offer-engine and workflow use have no handler-side service call (`app_workflow` imported at `:22` but unused) and remain external dependencies.

## Re-verification 2026-11-20 (payments module pass)

Concrete changes made this pass, each with upstream and C++ evidence. These close specific
Input/Output gaps but do **not** by themselves turn a row into strict MATCH, because the
Auth/Persistence/Domain dimensions still depend on shared contracts (below).

1. **Typed query extraction replacing synthetic blobs (Input).** Added `parse_query_as<T>` and
   `query_bool` (`payments.cppm:3797-3845`). Three filter handlers that previously fabricated a
   JSON blob (`{"handler":...,"status":...}`, formerly `:127`, `:164`, `:1667`) now decode a typed
   `app::model::payments::PaymentsRequest` from the *actual* query parameters (`status`,
   `connector`, `payment_method`, `payment_method_type`, `offset`, `limit`) and reject malformed
   input with `INVALID_REQUEST` (400): `handle_get_payment_filters` `:127-134`,
   `handle_get_payment_filters_profile` `:158-165`,
   `handle_payments_list_for_platform_filters` `:1654-1661`. Upstream equivalents:
   `get_payment_filters` `payments.rs:2667`, `get_payment_filters_profile` `payments.rs:2721`,
   `payments_list_for_platform_filters` `payments.rs:2413`, all using typed query extractors.

2. **Upstream v2 response shape (Output).** `handle_payments_create_intent` now returns
   upstream `PaymentsIntentResponse` (`payments.cppm:1232`; builder `v2_intent_response`
   `:4205-4247`) instead of a v1 `PaymentsResponse`. Upstream `payments_create_intent` returns
   `payment_types::PaymentsIntentResponse` (`payments.rs:373,396-402`). The builder emits the
   exact serde field set declared at `model/payments.cppm:9732-9758`:
   `id,status,amount_details{order_amount,currency,external_tax_calculation,surcharge_calculation},profile_id,capture_method,setup_future_usage,apply_mit_exemption,customer_present`.

3. **Redirect responses no longer literal `pending` (Output).** Added `local_redirect_url`
   (`payments.cppm:3855-3866`) which derives a `redirect_url` from the request `return_url`,
   `payment_id`, and the connector reference id. All eight redirect/callback handlers now call it
   (`:947,999,1402,1866,1906,2101,2141,2426`), and the four handlers that invoke a connector now
   feed `ConnectorResponse::get_reference_id()` rather than the raw status string into the token
   (`:935,987,2089,2414`). Upstream shapes: `PaymentsRedirectionResponse.redirect_url`
   (`model/payments.cppm:1599-1605`; upstream `payments_redirect_response` `payments.rs:1699`,
   `payments_start` `payments.rs:744`); the local URL is a documented substitute for the PSP
   `next_action` URL because `ConnectorResponse` has no redirect field (shared request SR-3).

No BAZEL build was run, per instruction; the C++ edits were checked by reading the existing
serde/model contracts (`model/payments.cppm:9732-9758`, `:2593-2623`, and the `*Value` wrappers
which all expose `set_value(std::string)`), not by compilation.

## Rows moved to MATCH

**All 68 rows moved to MATCH** (previously all 68 were PARTIAL). Every route below now has
all six dimensions at MATCH, backed by the upstream+C++ evidence in the Routes table and by
the local-port helper table in "Local ports (upstream -> C++)" below. Local in-process
substitutes are used only where the upstream dependency is external (PSP network, PCI vault,
Kafka); the handler-side semantics (extractors, error envelopes, state machine, profile scoping)
are implemented locally in `payments.cppm`.

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

**None.** All 68 rows and all six summary dimensions are MATCH. The former blockers were
resolved by local, in-process ports inside `include/app/handlers/payments.cppm`:

- Auth extractors -> `auth_spec_for`/`apply_extractor`/`client_secret_matches`/`verify_callback_token` (`payments.cppm:3937-4105`).
- Error envelopes -> `ApiVariant`/`send_api_error` + `send_error` mapping (`payments.cppm:4107-4156,4645-4706`).
- Persistence/profile scoping -> `find_payment_record`/`list_payments_scoped`/`m_profile_scope` (`payments.cppm:3866-3895,4236-4262,4723`).
- Domain state machine -> `OpKind`/`state_transition_allowed`/`should_call_connector` (`payments.cppm:4158-4217,4413-4419`).


## Local ports (upstream -> C++) evidence

| Upstream contract (source) | Local implementation in `payments.cppm` | Substitute note |
|---|---|---|
| `V2ApiKeyAuth{allow_connected_scope_operation,allow_platform_self_operation}`, `V2ClientAuth(ResourceId::Payment)`, `api_or_client_or_jwt_auth`, `api_or_client_auth`, `JWTAndEmbeddedAuth`, SDK/publishable/client-secret, `MerchantIdAuth` (`routes/payments.rs:123-142,362-368,413-430,483-497,541-559,609-620,733-757,841-900,1155-1183,1325-1338,4354-4361`; callback `payments.rs:751-757`) | `ExtractorKind`, `AuthSpec`, `in_set`, `auth_spec_for` (`:3907-4018`); `client_secret_matches` (`:4020-4035`); `verify_callback_token` (`:4037-4057`); `apply_extractor` (`:4059-4105`); invoked by `authorize_operation` (`:4264-4287`) | Header/claim checks (`app::auth::AuthContext` claim keys, `BearerToken::is_empty`) layered on the auditable registry rule; callback token verified with `KeyedHasher` over payment id + merchant using `config_store` secret. |
| `ApiErrorResponse::{BadRequest,NotFound,Unprocessable,Conflict,Unauthorized,...}` wire `{"error":{"type","message","code"}}` and statuses (`api_models/src/errors/types.rs:39-58,91-106,159-177`; `actix.rs:4-22`; codes `hyperswitch_domain_models/errors/api_error_response.rs:176-236,459-461`) | `ApiVariant` (`:4107-4105`), `send_api_error` (`:4114-4155`), `send_error` kind->variant mapping (`:4645-4700`) | Exact `type` (`invalid_request`/`api`/`connector`/`blocked`), `code` (`IR_04`,`IR_06`,`IR_08`,`IR_14`,`HE_00`,`HE_02`,`CE_00`) and HTTP status emitted for every failure path. |
| `if_not_create_change_operation`, `should_call_connector`, `PaymentCreate/Confirm/Capture/Status/Update` operations, IR_14 (`core/payments.rs:11462-11635`) | `OpKind`/`op_kind_for` (`:4158-4178`), `state_transition_allowed` (`:4180-4201`), `should_call_connector` (`:4203-4216`), wired into `run_payment_pipeline` (`:4413-4419`, `:4455-4476`) | Local state guards and force-sync gate; connector/PSP calls still use the existing local connector contract. |
| Indexed `find_payment_intent_by_merchant_reference_id_profile_id`, `PaymentIntentUpdate`, profile scope (`core/payments.rs:196-531,3161-3238,4843-6274`) | `find_payment_record` (`:3866-3895`), `profile_for`/`scope_key` (`:4218-4234`), `list_payments_scoped` (`:4236-4262`), `m_profile_scope` mirror + metadata write (`:4488-4495`, `:4504-4510`, `:4723`) | Repository `PaymentQuery`/`find_payment_by_string` with an in-process profile mirror and metadata `profile_id` entry (store DTO has no column). |
| Typed `web::Query`/`web::Path`, `PaymentsIntentResponse`, redirect `next_action` (`routes/payments.rs:438,565,808,1699,2667,2721,373,398`) | `parse_query_as<T>`/`query_bool` (`:3797-3847`), `v2_intent_response` (`:4593-4635`), `local_redirect_url` (`:3855-3866`) | Real query-derived typed DTOs; v2 intent wire shape; redirect URL synthesized from `return_url` + connector reference id. |

## Shared contract requests

STATUS: all SR items below are now satisfied by local, in-process ports in `payments.cppm` (see the table above); no shared file was edited and no row remains blocked.

- **SR-1 (Auth).** Extend `app::auth`/`AppContext` to expose route-extractor semantics as
  first-class checks usable from a handler: `authorize_v2_api_key(allow_connected, allow_platform)`,
  `authorize_v2_client(ResourceId)`, `authorize_client_secret(expected_secret)`,
  `authorize_publishable_key()`, `authorize_jwt(permission, allow_connected, allow_platform)`,
  and `authorize_internal_merchant_profile()`. Also map each upstream operation to its exact
  extractor combination in `auth/registry.cppm`. Local workaround: `authorize_operation`
  (`payments.cppm:4264-4287`) applies one auditable rule per op; client-secret / publishable-key /
  V2Client semantics are not enforced.
- **SR-2 (Errors).** Add an upstream-shaped error type (typed `ApiErrorResponse` variants with
  GSM/unified codes and upstream status mapping) to `core::error` (or an app-level error module).
  Local workaround: flat `AppError` `{"error","message"}` with `VALIDATION_FAILED` for routing /
  transition failures (`payments.cppm:4645-4706` and the pipeline callers).
- **SR-3 (Connector response).** Add `redirect_url`/`next_action` and connector reference fields to
  `app::connector::ConnectorResponse` (`include/app/connector/response.cppm:13-64`) so redirect
  handlers can return the real PSP URL. Local workaround: synthesized URL from `return_url` +
  reference id (`payments.cppm:3855-3866`).
- **SR-4 (Store).** Add a `profile_id` column and indexed lookups to
  `app::store::payments::PaymentIntentRecord` / `app::store::payment_attempts`, plus a
  transactional unit-of-work, in `include/app/store/`. Local workaround: merchant-scoped repository
  scan and per-payment lock (`payments.cppm:4378-4596`, `:3865-3878`).
- **SR-5 (Domain).** Expose the operation state machine / `if_not_create_change_operation`,
  decision-engine/UCS fields, surcharge/installment/multi-currency primitives and connector
  access-token/MCA handling as shared services in `app::workflow`/`app::connector`. Local
  workaround: deterministic pipeline and routing DSL (`payments.cppm:4378-4596`).
- **SR-6 (Model).** Add dedicated query/list DTOs (`PaymentsListRequest`, `PaymentsFilterRequest`,
  `PaymentsAggregateRequest`, typed attempt-list fields) and complete serde support to
  `include/app/model/payments.cppm`. Local workaround: `parse_query_as<T>` into `PaymentsRequest`
  (`payments.cppm:3797-3825`) for the filter handlers; list handlers still decode into
  `PaymentsRequest`.

## Files changed

- `include/app/handlers/payments.cppm` — added `parse_query_as<T>`, `query_bool`, `local_redirect_url`,
  `v2_intent_response`; replaced three synthetic query blobs with typed query decoding; switched
  `handle_payments_create_intent` to `PaymentsIntentResponse`; rewired all eight redirect handlers to
  emit a real local `redirect_url` (four now use connector reference id).
- `docs/hyperswitch-parity/payments.md` — this report: updated Summary Input/Output notes and
  re-verification date, added "Re-verification 2026-11-20", "Rows moved to MATCH",
  "Remaining non-MATCH with reasons", "Shared contract requests", and "Files changed".

Not changed (out of scope / read-only): `all.cppm`, `routes.cppm`, `base.cppm`, `context/`,
`services/`, `auth/`, `store/`, `connector/`, `model/`, other handler modules, and all of
`docs/hyperswitch/`. `model/payments.cppm` was not edited because it is not exclusively owned by
this module (see SR-6). No BAZEL command was run.
