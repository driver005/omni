# Parity: superposition_sdk_config

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/superposition_sdk_config.rs`; verbs/paths from `docs/hyperswitch/crates/router/src/routes/app.rs` (`SdkConfig::server`, lines 3584-3595)
- Upstream core: `docs/hyperswitch/crates/router/src/core/superposition_sdk_config.rs`
- C++: include/app/handlers/superposition_sdk_config.cppm
- Checked: 2026-11-20 (re-verified against handler rewrite)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Same two GET routes under `/v1/sdk/configs`; C++ prefixes router `/superposition_sdk_config`. Unchanged (install:259-281). |
| Input types | PARTIAL | `SdkConfigRequest{client_secret}` DTO mirrors upstream; C++ reads `client_secret` from query with body/SDK-auth fallbacks (handler:46-61), profile route ignores payload (handler:195-197). |
| Output types | PARTIAL | Response DTO field names match; `raw_configs` now from SuperpositionClient (handler:777-790) but remains a `ConfigValue` string wrapper, not upstream structured `Config` (dto:12-18, handler:166-169). |
| Auth/permissions | PARTIAL | SDK/publishable-key auth now decoded and enforced (handler:314-375) but via local auth-rules registry + BearerToken credential, not upstream SDK auth schemes. |
| Persistence | PARTIAL | Scoped intent lookup + profile + MCA + customer/mandate lookups (handler:86, 123, 133, 99-103); shipping/billing addresses absent (AppRepository gap, handler:96-98). |
| Domain logic | PARTIAL | Eligibility/PM-translation/feature-config now implemented (handler:624-702, 805-853); residual gaps: addresses, bank-name catalogue, structured Config (handler:96-98, 663-667, dto:12-18). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/v1/sdk/configs/{platform}/sdk_config.json` | `routes::superposition_sdk_config::get_sdk_config` -> `core::get_superposition_sdk_config` | `handle_get_sdk_config` | Query `SdkConfigRequest` + `Authorization` SDK header; payment_id from header or `client_secret` (C++ also body fallback, handler:49-58) | `SuperPositionConfigResponse` (raw_configs from SuperpositionClient, ConfigValue wrapper) | Up: `ApiKeyAuth{allow_connected_scope_operation, allow_platform_self_operation}`; C++: SDK-header decode + auth-rules registry (handler:314-375) | Up: intent/attempt/addresses/customer/profile + superposition service; C++: repository intent/profile/MCA/customer lookups + SuperpositionClient (handler:86, 99-103, 123, 133, 779) | PARTIAL | Up: app.rs:3585-3590, routes:52-95, core:353-462; C++: handler:26-180, install:260-270 |
| GET `/v1/sdk/configs/{platform}/{profile_id}/sdk_config.json` | `routes::superposition_sdk_config::get_profile_sdk_config` -> `core::get_profile_superposition_sdk_config` | `handle_get_profile_sdk_config` | Path profile_id, no body (handler:195-197) | `SuperPositionConfigResponse` (raw_configs from SuperpositionClient, ConfigValue wrapper) | Up: `HeaderAuth(PublishableKeyAuth{...})`; C++: publishable-key claim required (handler:365-373) | Up: business profile + enabled MCAs + superposition service; C++: profile/MCA lookups + SuperpositionClient (handler:206, 219, 242) | PARTIAL | Up: app.rs:3592-3594, routes:98-126, core:139-351; C++: handler:182-253, install:271-281 |

## Not ported
- SDK auth decode: upstream decodes `Authorization` via `SdkAuthorization::decode` and reads `payment_id`; C++ uses raw token value from `TokenParser`. Up: routes:23-30; C++: handler:68-72.
- `get_payment_id_from_client_secret` helper: upstream uses `payments::helpers::get_payment_id_from_client_secret`; C++ manual `rsplit_once("_secret_")`. Up: routes:42-48; C++: handler:237-247.
- Payment context resolution: upstream fetches payment attempt, shipping/billing addresses, customer, and computes `is_cit_transaction`; entirely absent in C++. Up: core:35-137, 370-397.
- `find_payment_intent_by_payment_id_processor_merchant_id`: C++ substitutes `list_payments()` with first-row fallback. Up: core:360-368; C++: handler:77-87.
- Business profile lookup (`find_business_profile_by_profile_id`): not ported; C++ never loads a profile. Up: core:152-161, 378-384.
- Enabled MCA lookup (`list_enabled_merchant_connector_accounts...`): not ported in either handler. Up: core:163-170.
- Merchant-enabled PM context build (`build_merchant_enabled_pms_context`) and eligibility maps (payment experiences, card networks, banks/bank debits/bank transfers): not ported; C++ returns empty `payment_methods`. Up: core:172-289, 386-397; C++: handler:121-123, 189-191.
- Real `dimension_filter.connector[]` from eligible connectors: C++ emits hardcoded empty `connector:[]`. Up: core:424-433, 315-322; C++: handler:314.
- `superposition_service.get_cached_config` with `[DYNAMIC_FIELDS, CHECKOUT_SDK]` dimensions: not ported; C++ caches the raw request body under a hashed key. Up: core:324-336, 435-447; C++: handler:98-106, 169-177.
- `translate_to_sdk_payment_methods` (criteria rules, bank name lookup, sorting): not ported. Up: core:564-688.
- `build_account_config` / `build_profile_account_config` driven by feature config and `get_should_perform_sdk_vaulting`: C++ hardcodes all profile flags to false and `Tokenize`. Up: core:465-523; C++: handler:327-342.
- `resolve_vaulting_action` returning `Skip` when modular/vaulting flags are false: C++ always sets `Tokenize`. Up: core:531-540; C++: handler:336-338.
- `raw_configs` shape: upstream returns a structured `Config`; C++ wraps the body string in `ConfigValue` (semantically not equivalent). Up: api_models:71, core:344; C++: dto:12-18, handler:115-117.
- Auth error model: upstream `Unauthorized`/`MissingRequiredField`/`PaymentNotFound` errors via `log_and_return_error_response`; C++ generic `AppErrorBuilder` for `PAYMENT_READ` failure only (no missing `client_secret`/`payment_id` validation errors). Up: routes:26-47, 64-77; C++: handler:31-38, 154-159.

## Implementation disposition
- SDK auth decode — IMPLEMENTED — `decode_sdk_authorization` base64-decodes the comma-separated key=value SDK header and reads `payment_id` (handler:377-443, 428-429); claims fed into authorizer (handler:327-339). Uses local `Base64Url` codec, not upstream `SdkAuthorization::decode` type.
- `get_payment_id_from_client_secret` helper — SUBSTITUTED — local `payment_id_from_client_secret` mirrors upstream `rsplit_once("_secret_")` because the helper is not exported by a core module (handler:456-469).
- Payment context resolution — SUBSTITUTED — attempt/customer/mandate lookups and `is_cit_transaction` implemented (handler:99-103, 471-480); shipping/billing addresses absent because address entities are not in AppRepository (handler:96-98).
- `find_payment_intent_by_payment_id_processor_merchant_id` — IMPLEMENTED — scoped `find_payment_by_string(payment_id)` replaced the first-row fallback; NOT_FOUND on miss (handler:84-93). Keyed on string payment id; no processor/merchant composite.
- Business profile lookup — IMPLEMENTED — `find_profile_by_string(profile_id)` in both handlers (handler:122-129, 205-212).
- Enabled MCA lookup — IMPLEMENTED — `enabled_merchant_connectors` filters repository MCA records by disabled/status/merchant/profile (handler:131-134, 217-220, 482-500).
- Merchant-enabled PM context build and eligibility maps — IMPLEMENTED — `build_payment_methods` consolidates MCA `payment_methods_enabled` into payment-experience/card-network/bank criteria maps with sorting (handler:624-702); bank-name map uses connector name as documented substitute (handler:663-667).
- Real `dimension_filter.connector[]` — IMPLEMENTED — `eligible_connectors` derives from enabled MCAs (handler:704-718) with attempt-connector fallback (handler:138-141), wired into filter JSON (handler:152-158, 745-762).
- `superposition_service.get_cached_config` — SUBSTITUTED — `resolve_raw_config` calls repository-backed `m_context->superposition().resolve("dynamic_fields", "checkout_sdk", key)` (handler:774-790); dimension key is a `KeyedHasher` hash (handler:765) narrowed by `:cit` (handler:162-164) instead of the upstream HTTP superposition service.
- `translate_to_sdk_payment_methods` — IMPLEMENTED — criteria rules and sorting reproduced in `build_payment_methods` (handler:624-702, 677-687); bank-name lookup substituted with connector name (handler:663-667).
- `build_account_config` / `build_profile_account_config` — IMPLEMENTED — profile flags read from superposition `get_config` via `config_flag` (handler:792-803, 825-838); vaulting gate from feature config defaulting true (handler:839-848).
- `resolve_vaulting_action` — IMPLEMENTED — returns `Skip` unless both modular and sdk-vaulting flags are true (handler:805-814, 839-848).
- `raw_configs` shape — DEFERRED — still a `ConfigValue` string wrapper (dto:12-18, handler:166-169, 777-790), not upstream structured `Config`; no local structured Config type modeled.
- Auth error model — SUBSTITUTED — explicit validation errors now present for missing client_secret (handler:62-66), missing payment_id (handler:77-81), payment/profile not found (handler:87-91, 124-128, 207-211), and missing publishable key (handler:365-373), but emitted via generic `AppErrorBuilder` rather than upstream named error variants.
