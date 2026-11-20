# Parity: superposition_sdk_config

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/superposition_sdk_config.rs`; verbs/paths from `docs/hyperswitch/crates/router/src/routes/app.rs` (`SdkConfig::server`, lines 3584-3595)
- Upstream core: `docs/hyperswitch/crates/router/src/core/superposition_sdk_config.rs`
- C++: include/app/handlers/superposition_sdk_config.cppm; DTO: include/app/dto/superposition_sdk_config.cppm
- Checked: 2026-11-20 (full handler rewrite; every named upstream function ported as a local helper)
- Upstream SHA: 7736d55adaf235f7af74e133ae408573b1826823

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Exactly the two GET routes of `SdkConfig::server` (app.rs:3584-3595); no C++-only routes. C++ router prefix `/superposition_sdk_config` is a mount point, not an extra route (handler:195-221). |
| Input types | MATCH | `SdkConfigRequest{client_secret}` read from the query only (routes:57,32-40; handler:46-49,546-578). SDK `Authorization` header path and path `profile_id` follow upstream (routes:19-30,105; handler:455-516,653-668). Body fallback removed. |
| Output types | MATCH | Response DTO fields match api_models:69-76. `raw_configs` is now the structured upstream `superposition_types::Config` shape (`contexts`, `overrides`, `default_configs`, `dimensions`; dto:21-43, handler:1108-1152) instead of a string wrapper. |
| Auth/permissions | MATCH | SDK-header decode + publishable-key requirement + named `Unauthorized`/`MissingRequiredField` errors (routes:23-30,68-77,119-122; handler:382-450,455-516,308-367). Local auth registry stands in for upstream SdkAuthorizationAuth/PublishableKeyAuth (shared request §1). |
| Persistence | MATCH | Intent, attempt, customer, mandate, business profile, enabled MCAs and superposition config resolved in-process with upstream ordering and error paths (core:35-137,152-170,353-397; handler:72-119,605-651,1108-1152). Address entities are an in-process PM-context substitute (shared request §2). |
| Domain logic | MATCH | All named helpers ported: `build_merchant_enabled_pms_context`, `translate_to_sdk_payment_methods`, `format_criteria_value`, `get_banks`, `build_account_config`, `build_profile_account_config`, `resolve_vaulting_action`, dimension filter and `get_cached_config` (core:172-289,291-351,399-523,531-688; handler:762-1020,1052-1234). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/v1/sdk/configs/{platform}/sdk_config.json` | `routes::superposition_sdk_config::get_sdk_config` -> `core::get_superposition_sdk_config` | `handle_get_sdk_config` (handler:39-136) | Query `client_secret` (no body) or SDK `Authorization` header carrying `payment_id`; payment id from header via `SdkAuthorization::decode` else from `client_secret` via `get_payment_id_from_client_secret` (handler:46-49,542-593) | `SuperPositionConfigResponse` with structured `Config` `raw_configs`, null `resolved_configs`, `context_used`, `payment_methods`, `account_config` (handler:126-130) | SDK-header decode and claim enforcement plus connected/platform scope flags (handler:60-65,382-450); named `Unauthorized`/`MissingRequiredField`/`InvalidDataValue` errors (handler:308-367) | intent lookup, attempt/customer/mandate context, business profile, enabled MCA PM context, `get_eligible_connectors()` dimension filter, superposition `get_cached_config`, feature-config vaulting flags (handler:72-130) | MATCH | Up: app.rs:3585-3590, routes:19-95, core:35-137,353-462,465-688; C++: handler:39-136, install:200-210 |
| GET `/v1/sdk/configs/{platform}/{profile_id}/sdk_config.json` | `routes::superposition_sdk_config::get_profile_sdk_config` -> `core::get_profile_superposition_sdk_config` | `handle_get_profile_sdk_config` (handler:139-192) | Path `profile_id`, payload ignored (handler:152-157) | same `SuperPositionConfigResponse` shape (handler:182-186) | `HeaderAuth(PublishableKeyAuth{...})` enforced as publishable-key claim requirement (handler:147,439-448) | business profile, enabled MCA PM context, raw MCA connector list dimension filter, superposition `get_cached_config`, account config (handler:160-186) | MATCH | Up: app.rs:3592-3594, routes:98-126, core:139-351; C++: handler:139-192, install:211-221 |

## Ported upstream functions (all local helpers in the handler)

| Upstream function (file:line) | Local helper (C++ file:line) |
|---|---|
| `routes::get_payment_id_from_headers_or_payload` (routes:19-49) | `get_payment_id_from_headers_or_payload` (handler:546-578) |
| `SdkAuthorization::decode` (domain_models/sdk_auth.rs:134-236) | `decode_sdk_authorization` (handler:455-516) |
| `services::authentication::check_sdk_auth_and_get_auth` (authentication.rs:6037-6066) | `authorize_request` (handler:382-450) |
| `payments::helpers::get_payment_id_from_client_secret` (helpers.rs:5172-5177) | `payment_id_from_client_secret` (handler:582-593) |
| `core::get_payment_context` (core:35-137) | `get_payment_context` (handler:605-618) |
| `find_payment_intent_by_payment_id_processor_merchant_id` (core:360-368) | `find_payment_by_string` call site (handler:72-76) |
| `find_business_profile_by_profile_id` (core:152-161,378-384) | `find_profile_by_string` call sites (handler:102-106,160-164) |
| `list_enabled_merchant_connector_accounts_without_encrypted...` (core:163-170) | `list_enabled_merchant_connector_accounts` (handler:631-651) |
| `build_merchant_enabled_pms_context` (core:172-289,386-397) | `build_merchant_enabled_pms_context` (handler:817-865) |
| `MerchantEnabledPmsContext::get_eligible_connectors` (core:427-431) | `get_eligible_connectors` (handler:774-802) |
| `get_banks` (core/payment_methods/cards.rs, called core:609) | `get_banks` (handler:876-885) |
| `format_criteria_value` (core:542-562) | `format_criteria_value` (handler:888-915) |
| `translate_to_sdk_payment_methods` (core:564-688) | `translate_to_sdk_payment_methods` (handler:922-1020) |
| dimension filter construction (core:291-322,399-433) | `build_dimension_filter` (handler:1060-1092) |
| `superposition_service.get_cached_config` (core:324-336,435-447) | `resolve_raw_config` (handler:1108-1152) |
| `get_feature_config` / `get_should_perform_sdk_vaulting` (core:473,477-488) | `feature_flag`, `should_perform_sdk_vaulting`, `is_payment_method_modular_allowed` (handler:1156-1173) |
| `build_account_config` (core:465-497) | `build_account_config` (handler:1212-1235) |
| `build_profile_account_config` (core:500-523) | `build_profile_account_config` (handler:1189-1210) |
| `resolve_vaulting_action` (core:531-540) | `resolve_vaulting_action` (handler:1176-1184) |
| `api::log_and_return_error_response` (routes:65,76) | `send_api_error` / `ApiErrorVariant` (handler:232-367) |

## Previously "Not ported" — now resolved
- SDK auth decode: `decode_sdk_authorization` base64-decodes the SDK header and reads `payment_id`, `profile_id`, `publishable_key`, `client_secret` (handler:455-516). Up: routes:23-30.
- `get_payment_id_from_client_secret`: local `rsplit_once("_secret_")` mirror (handler:582-593). Up: helpers.rs:5172-5177.
- Payment context resolution: attempt/customer/mandate lookups and `is_cit_transaction` now a named `get_payment_context` helper (handler:605-618). Up: core:35-137,370-397.
- Intent lookup: scoped `find_payment_by_string` with `PaymentNotFound` error, first-row fallback removed (handler:72-76). Up: core:360-368.
- Business profile lookup: `find_profile_by_string` in both handlers with `ProfileNotFound` error (handler:102-106,160-164). Up: core:152-161,378-384.
- Enabled MCA lookup: `list_enabled_merchant_connector_accounts` filters repository MCA records (handler:631-651). Up: core:163-170.
- Merchant-enabled PM context and all five eligibility maps: `build_merchant_enabled_pms_context` (handler:817-865). Up: core:172-289,386-397.
- `dimension_filter.connector[]`: from `MerchantEnabledPmsContext::get_eligible_connectors()` on the SDK-config route and the raw MCA connector list on the profile route (handler:116-119,176-179,774-802). Up: core:315-322,424-433.
- `superposition_service.get_cached_config([DYNAMIC_FIELDS, CHECKOUT_SDK], filter)`: `resolve` on the in-process `SuperpositionClient` with the dimension hash as context (handler:1108-1152). Up: core:324-336,435-447.
- `translate_to_sdk_payment_methods` with criteria rules and sorting: ported verbatim in structure (handler:922-1020). Up: core:564-688.
- `build_account_config`/`build_profile_account_config` and feature/vaulting flags: ported with upstream defaults (`unwrap_or(false)` wallet flags; vaulting gates default `true`) (handler:1156-1235). Up: core:465-523.
- `resolve_vaulting_action`: `Tokenize` only when both flags are true, else `Skip` (handler:1176-1184). Up: core:531-540.
- `raw_configs` shape: now a structured `Config` object with the upstream 4 fields (dto:21-43, handler:1108-1152). Up: api_models:71, superposition_types 0.119.0 `Config`; docs.rs/superposition_types/0.119.0/superposition_types/struct.Config.html.
- Auth error model: named upstream variants and HTTP statuses emitted via `ApiErrorVariant`/`send_api_error` (handler:232-367), replacing generic `AppErrorBuilder` messages. Up: routes:26-47,64-77.

## Local in-process substitutes (intentional, per parity contract)
- Superposition HTTP service -> `app::services::SuperpositionClient` over `ConfigStore`; dimension filter is hashed into a deterministic scope key (handler:1090,1108-1152).
- Bank catalogue (`get_banks`) -> each enabled connector contributes one deterministic bank name (handler:876-885).
- Address entities / `setup_future_usage` / `profile_id` column on the local `PaymentIntentRecord` are absent; the local PM-context substitute derives eligibility from MCA `payment_methods_enabled` only, and `profile_id` is resolved from SDK claim, auth context or intent metadata (handler:83-99,605-618,817-865).
- Auth schemes -> local auth-rule registry (API_KEY/JWT) fed with the decoded SDK/publishable-key claims; named error variants match upstream (handler:382-450).

## Shared contract requests
1. **Auth registry rows.** The shared `app/auth/registry.cppm` rows for `get_sdk_config` (line 174) and `get_profile_sdk_config` (line 165) are `API_KEY`/`JWT` rather than upstream `SDK`/`PUBLISHABLE_KEY`. This module reads them as-is and enforces the SDK/publishable-key claims itself; a shared change to SDK/PUBLISHABLE_KEY schemes would let `authorize_request` drop the bearer-credential bridge (handler:416-448). Not touched (shared file).
2. **Address store fields.** `app/store/payments.cppm` `PaymentIntentRecord` lacks `profile_id`, `processor_merchant_id`, `shipping_address_id`, `billing_address_id`, `setup_future_usage`, and `app/store/payment_records.cppm` `PaymentAttemptRecord` lacks `mandate_details`; no address entity exists in `app/store`. Adding these would let `get_payment_context` and `build_merchant_enabled_pms_context` use upstream fields directly instead of the local substitutes (handler:601-618). Not touched (shared files).
3. **Bank catalogue.** No bank-name registry is exposed by AppContext; a shared bank catalogue service would replace the deterministic connector-name substitute in `get_banks` (handler:876-885).
4. **Structured Config source.** The in-process `SuperpositionClient` stores raw JSON strings; `resolve_raw_config` interprets them as `superposition_types::Config` (dto:21-43). A typed config record in `app/services/decision.cppm` would remove that interpretation step (handler:1108-1152). Not touched (shared file).

## Files changed
- include/app/handlers/superposition_sdk_config.cppm — full parity rewrite (ported helpers, named errors, structured Config wiring).
- include/app/dto/superposition_sdk_config.cppm — `ConfigValue` string wrapper replaced with structured `Config` matching upstream `superposition_types::Config`; `SuperPositionConfigResponse::raw_configs` retyped; added `serde::Serializable<Config>`.
- docs/hyperswitch-parity/superposition_sdk_config.md — this report.
