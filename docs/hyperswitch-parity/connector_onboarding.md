# Parity: connector_onboarding

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/connector_onboarding.rs` (`get_action_url` `:11-32`, `sync_onboarding_status` `:34-55`, `reset_tracking_id` `:57-78`); registration `web::scope("/connector_onboarding")` with three `POST` resources in `docs/hyperswitch/crates/router/src/routes/app.rs:3366-3383` (gated `#[cfg(all(feature = "olap", feature = "v1"))]`).
- Upstream core: `docs/hyperswitch/crates/router/src/core/connector_onboarding.rs` (`get_action_url` `:20-58`, `sync_onboarding_status` `:60-120`, `reset_tracking_id` `:122-138`), helpers `docs/hyperswitch/crates/router/src/utils/connector_onboarding.rs:17-154`, PayPal flow `docs/hyperswitch/crates/router/src/core/connector_onboarding/paypal.rs:18-201`, PayPal types `docs/hyperswitch/crates/router/src/types/api/connector_onboarding/paypal.rs:1-245`.
- C++: include/app/handlers/connector_onboarding.cppm (handler), include/app/dto/connector_onboarding.cppm (module-named typed request/response DTOs).
- Checked: 2026-11-20

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | All three `POST /connector_onboarding/{action_url,sync,reset_tracking_id}` match (`app.rs:3369-3381` vs `cppm:300-333`). No C++-only routes exist. |
| Input types | MATCH | Typed DTO decode (`cppm:107,164,254`) into module-named DTOs whose id/connector fields are serde-transparent plain strings (`dto:85-130`), mirroring `web::Json<ActionUrlRequest/OnboardingSyncRequest/ResetTrackingIdRequest>` (`api_models/connector_onboarding.rs:5-10,18-24,59-63`; `common_utils/src/id_type.rs:88-104`; `common_enums/src/connector_enums.rs:29-32`). Upstream id character validation reproduced as `valid_id()` (`cppm:989-994`; `id_type.rs:74-77`). |
| Output types | MATCH | Externally-tagged lowercase `{"paypal":{...}}` action URL (`cppm:145-149`), `{"paypal":{"connector_integrated":<MCA>}}` (`cppm:231`), unit variants `{"paypal":"<snake_case>"}` (`cppm:234`) and the full `OnboardingStatus::PayPal` variant set (`cppm:64-69,799-821`) exactly mirror `api_models:12-16,31-47`. `StatusOk` is an empty 200 body (`cppm:292`). |
| Auth/permissions | MATCH | JWT + `MERCHANT_WRITE` + `allow_connected=true` + `allow_platform=false` mirrors `JWTAuth{MerchantAccountWrite,...}`; connected-merchant claim scopes merchant id (`cppm:94-99,162-166,248-251,434-443` vs `routes:24-28,47-51,70-74`; `app.rs:3366`). |
| Persistence | MATCH | Tracking id is an upsert/read on the real `configs` DB table through `AppRepository` with upstream key `onboarding_paypal_<connector_id>` (`cppm:528-579` vs `utils:85-154`; `consts.rs:107`), and upstream `<connector_id>_<unix_timestamp>` mint. MCA update persists `connector_type=payment_processor`, `connector_account_details` (SignatureKey JSON), `disabled=false`, `status=active` (`cppm:883-918` vs `paypal:151-170`). Vault encryption is the documented local in-process substitute (`cppm:904-909`). |
| Domain logic | MATCH | Every named upstream helper is ported as a local helper with upstream logic: `get_connector_auth`, `is_enabled`, `check_if_connector_exists`, `validate_profile_id_from_auth_layer`, `build_key`, `set_tracking_id_in_configs`, `get_tracking_id_from_configs`, `build_referral_url`, `build_referral_request`, `get_action_url_from_paypal`, `generate_access_token`, `extract_action_url`, `merchant_onboarding_status_url`, `find_paypal_merchant_by_tracking_id`, `extract_merchant_details_url`, `get_eligibility_status`, `sync_merchant_onboarding_status`, `update_mca` (`cppm:450-918`). External PayPal HTTP / `admin::update_connector` are replaced by in-process connector/repository substitutes as authorized by the strict-parity brief. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/connector_onboarding/action_url` | `get_action_url` (olap/v1) | `handle_get_action_url` (`cppm:88`) | `web::Json<ActionUrlRequest>` typed decode (`cppm:107`) | `ActionUrlResponse::PayPal` external-tag (`cppm:145-149`) | `JWTAuth(MerchantAccountWrite, allow_connected, !allow_platform)` (`cppm:94-99`) | `check_if_connector_exists` → `is_enabled` → `get_tracking_id_from_configs` → `(Some(true),Paypal)` → `get_action_url_from_paypal` (`cppm:123-145`); NotImplemented for non-PayPal in `get_connector_auth` (`cppm:450-458`) | MATCH | `routes:11-32`; `core:20-58`; `utils:17-43,45-83,125-154`; `paypal:18-63`; `api_models:5-16`; `cppm:88-153,450-713` |
| POST `/connector_onboarding/sync` | `sync_onboarding_status` (olap/v1) | `handle_sync_onboarding_status` (`cppm:156`) | `web::Json<OnboardingSyncRequest>` typed decode; body `profile_id` ownership check (`cppm:164,193`) | `OnboardingStatus::PayPal` variants; `ConnectorIntegrated(MCA)` on Success (`cppm:216-237`) | same JWT rule (`cppm:162-166`) | `check_if_connector_exists(Some(profile_id))` → config gate → tracking id → `sync_merchant_onboarding_status` → `get_eligibility_status` precedence → SignatureKey `update_mca` (`cppm:193-231,799-918`); FlowNotSupported for non-PayPal (`cppm:207`) | MATCH | `routes:34-55`; `core:60-120`; `utils:17-43,45-83,125-154`; `paypal:81-115,117-142,144-201`; `types/paypal:188-244`; `core/utils.rs:2889-2912`; `api_models:31-47`; `cppm:156-239,450-523,612-918` |
| POST `/connector_onboarding/reset_tracking_id` | `reset_tracking_id` (olap/v1) | `handle_reset_tracking_id` (`cppm:242`) | `web::Json<ResetTrackingIdRequest>` typed decode (`cppm:254`) | `StatusOk` empty 200 (`cppm:292`) | same JWT rule (`cppm:248-251`) | `check_if_connector_exists(None)` → `set_tracking_id_in_configs` upsert on `configs` (`cppm:278-288,545-566`); no enabled/connector-type gate, per upstream | MATCH | `routes:57-78`; `core:122-138`; `utils:45-83,85-123,144-154`; `app.rs:3379-3382`; `cppm:242-292,528-566` |

## Ported upstream functions (local helpers)
Every function named in the original `## Not ported` list is now implemented in the handler; none remain unported.

| Upstream function | Upstream evidence | C++ helper | C++ evidence |
|---|---|---|---|
| `utils::get_connector_auth` | `utils/connector_onboarding.rs:17-33` | `get_connector_auth` (BodyKey for PayPal, `not_implemented` otherwise) | `cppm:450-458,361-368` |
| `utils::is_enabled` | `utils:35-43` | `is_enabled` (`std::optional<bool>`) | `cppm:462-470` |
| `utils::check_if_connector_exists` | `utils:45-83` | `check_if_connector_exists` | `cppm:495-523` |
| `core::utils::validate_profile_id_from_auth_layer` | `core/utils.rs:2889-2912` | `validate_profile_id_from_auth_layer` (three-case match, IR_16/400) | `cppm:474-489,382-389` |
| `utils::build_key` | `utils:144-154`; `consts.rs:107` | `build_key` (`onboarding_<connector>_<id>`) | `cppm:528-534` |
| `utils::set_tracking_id_in_configs` | `utils:85-123` | `set_tracking_id_in_configs` (`configs` DB upsert) | `cppm:545-566` |
| `utils::get_tracking_id_from_configs` | `utils:125-142` | `get_tracking_id_from_configs` | `cppm:570-580` |
| `paypal::build_referral_url` | `core/.../paypal.rs:18-23` | `build_referral_url` | `cppm:584-587` |
| `PartnerReferralRequest::new` | `types/.../paypal.rs:112-147` | `build_referral_request` | `cppm:590-607` |
| `utils::paypal::generate_access_token` | `utils/.../paypal.rs:17-40` | `generate_access_token` (calls `get_connector_auth`) | `cppm:612-641` |
| `PartnerReferralResponse::extract_action_url` | `types/.../paypal.rs:235-244` | `extract_action_url` | `cppm:644-670` |
| `paypal::get_action_url_from_paypal` | `core/.../paypal.rs:40-63` | `get_action_url_from_paypal` | `cppm:675-714` |
| `paypal::merchant_onboarding_status_url` | `core/.../paypal.rs:65-79` | `merchant_onboarding_status_url` | `cppm:716-725` |
| `SellerStatusResponse::extract_merchant_details_url` | `types/.../paypal.rs:177-186` | `extract_merchant_details_url` | `cppm:728-739` |
| `paypal::find_paypal_merchant_by_tracking_id` | `core/.../paypal.rs:117-142` | `find_paypal_merchant_by_tracking_id` | `cppm:742-797` |
| `SellerStatusDetailsResponse::get_eligibility_status` | `types/.../paypal.rs:188-232` | `get_eligibility_status` (exact precedence) | `cppm:799-821` |
| `paypal::sync_merchant_onboarding_status` | `core/.../paypal.rs:81-115` | `sync_merchant_onboarding_status` | `cppm:823-877` |
| `paypal::update_mca` | `core/.../paypal.rs:144-201` | `update_mca` (SignatureKey auth JSON) | `cppm:883-918` |
| core `get_action_url` / `sync_onboarding_status` / `reset_tracking_id` | `core:20-58/60-120/122-138` | route handlers in upstream call order | `cppm:88-154/156-239/242-292` |

## Local in-process substitutes for external infra
- **PayPal partner-referral / merchant-integration HTTP** (`paypal:25-63,95-142`): the injected `IConnector` (`exchange_token`/`create_link`/`sync`) stands in for the HTTP client; request bodies are built with upstream `PartnerReferralRequest::new` field-for-field (`cppm:590-607`) and all responses pass through upstream `extract_action_url` / `extract_merchant_details_url` / `get_eligibility_status` (`cppm:644-670,728-739,799-821`). No real network call or PayPal credentials exist locally.
- **`admin::update_connector`** (`paypal:188-200`): MCA update goes through `AppRepository::update_merchant_connector` with the upstream `MerchantConnectorUpdate` fields (`cppm:906-909`).
- **Vault encryption of `Secret<serde_json::Value>`** (`paypal:151-158`): `AesWrap` + a deterministic key derived from the merchant/connector material stands in for the upstream vault (`cppm:904-909,948-960`).
- **Merchant key store** (`utils:51-70`): the local `KeyStore` has no `get_master_key`/`get_merchant_key_store_by_merchant_id`; wrap material is read from `key_store().get("merchant_key", merchant_id)` and existence is established through `AppRepository`, as documented at `cppm:495-523`.
- **`server_wrap` / `Flow` / `#[instrument]` / `LockAction::NotApplicable`** (`routes:16-31,39-53,62-76`): framework scaffolding has no C++ analogue; the three `Flow` metric labels are still recorded at `cppm:151-152,237-238,289-290`. olap/v1 feature gating is compile-time upstream and unconditional here.

## Implementation disposition (re-verified 2026-11-20)
- `api::server_wrap` scaffolding — MATCH (substitute) — Flow metrics at `cppm:151-152,237-238,289-290`; no C++ locking/feature-gate analogue (see above).
- `is_enabled` config gate — MATCH — `is_enabled` returns `Some(conf.paypal.enabled)` for PayPal / `None` otherwise (`cppm:462-470`); enforced through the upstream `(Some(true), Paypal)` match in both action URL and sync (`cppm:129-137,200-208`).
- Real PayPal partner-referral HTTP — MATCH (local substitute) — `get_action_url_from_paypal` (`cppm:675-714`) with upstream request shape and response extraction.
- Real PayPal onboarding-status state machine — MATCH — `sync_merchant_onboarding_status` (`cppm:823-877`) + exact `get_eligibility_status` precedence (`cppm:799-821`): `payments_not_receivable`, `email_not_verified`, `ppcp_custom_denied`, `more_permissions_needed`, `success`; `account_not_found` from `find_paypal_merchant_by_tracking_id` (`cppm:834-837`).
- `update_mca` via `admin::update_connector` — MATCH (local substitute) — SignatureKey `{auth_type:"SignatureKey", api_key:client_secret, key1:client_id, api_secret:payer_id}` (`cppm:224-231,883-918`); stored through repository with upstream `MerchantConnectorUpdate` fields.
- DB-backed tracking id — MATCH — `onboarding_paypal_<connector_id>` key (`cppm:528-534`) on the `configs` repository table (`cppm:545-580`); upstream `<connector_id>_<unix_timestamp>` mint.
- Merchant key store + full MCA lookup — MATCH (local substitute) — `check_if_connector_exists` (`cppm:495-523`) plus ported `validate_profile_id_from_auth_layer` (`cppm:474-489`).
- `get_connector_auth` helper + `NotImplemented` — MATCH — `get_connector_auth` (`cppm:450-458`) emits upstream-shaped `{"error":"NotImplemented","message":"Onboarding is not implemented for <connector> is not implemented"}` with HTTP 501 (`cppm:361-368`). Upstream routes never reach it for non-PayPal (they return FlowNotSupported/400 at the core match first, `cppm:136,207`), exactly as upstream.
- Enum `#[serde(rename_all)]` wire envelopes — MATCH — `cppm:145-149,231,234,64-69`.
- Typed request DTOs with id wrappers — MATCH — typed decode at `cppm:107,164,254` into `dto:85-130`; id/connector newtypes are serde-transparent plain strings on the wire, so no `{"value": ...}` unwrap remains.

## Shared contract requests
These would require edits outside my allowed files; recording them here for the owning lanes. None block the MATCH verdict because in-process substitutes are implemented locally.
- `core/error`: add an `AppErrorKind::NOT_IMPLEMENTED` with HTTP 501 and wire name `NotImplemented`, so `LocalError`'s explicit status override (`cppm:32-36,338-395`) can be replaced by the shared kind.
- `app/store`: add typed `find_merchant_connector_account_by_merchant_id_and_merchant_connector_id` and key-store-aware decryption if the upstream `accounts_store` API is needed by other modules; I currently use `find_merchant_connector_by_string` + merchant/profile checks (`cppm:508-523`).
- `app/services::ConfigStore`: make `put`/`get` repository-backed instead of in-memory; I bypass it for tracking ids and use `AppRepository` `configs` directly (`cppm:545-580`).
- `app/connector`: add a PayPal-specific operation (or HTTP client wrapper) for `v2/customer/partner-referrals` and `v1/customer/partners/.../merchant-integrations`; today the in-process `create_link`/`sync` substitutes carry the same request/response shapes.
