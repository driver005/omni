# Parity: profile_acquirer

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/profile_acquirer.rs` (`create_profile_acquirer` `:13-50`, `profile_acquirer_update` `:54-97`); registration `web::scope("/profile_acquirer")` with `POST ""` and `POST /{profile_id}/{profile_acquirer_id}` in `docs/hyperswitch/crates/router/src/routes/app.rs:3520-3534`
- Upstream core: `docs/hyperswitch/crates/router/src/core/profile_acquirer.rs` (`create_profile_acquirer` `:12-102`, `update_profile_acquirer_config` `:105-215`, helpers `apply_default_bucket_change` `:219-236` and `upsert_acquirer_config_in_bucket` `:241-312`)
- C++: include/app/handlers/profile_acquirer.cppm
- Checked: 2026-11-20 (re-verified against current `profile_acquirer.cppm`)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Both expose `POST /profile_acquirer` and `POST /profile_acquirer/:profile_id/:profile_acquirer_id` (`app.rs:3528,3531-3532` vs `profile_acquirer.cppm:186-208`). |
| Input types | MATCH | Typed DTOs mirror upstream fields: create has 9 fields, update 8 fields including optional `network`/`is_default` (`api_models/profile_acquirer.rs:5-32,156-174` vs `dto/profile_acquirer.cppm:179-195,251-266`, used at `profile_acquirer.cppm:41-42,101-102`). |
| Output types | PARTIAL | Both emit `ProfileAcquirerResponse` shape. C++ now builds response from the re-read stored config with normalized country (`:133-136,249,394-423,464-489`) instead of echoing raw fields, but through a local `StoredConfig` substitute (`:29-75`), so values can still differ from upstream `AcquirerConfig` (`core:94-99,207-212`). |
| Auth/permissions | PARTIAL | Upstream `HeaderAuth(ApiKeyAuth)` or `JWTAuthProfileFromRoute` with `Permission::ProfileAccountWrite` bound to route `profile_id` (`profile_acquirer.rs:34-46,81-93`); C++ resolves an `auth_rules` rule via `authorize_route` (`profile_acquirer.cppm:88-90,151-153`), still no route-profile binding. |
| Persistence | PARTIAL | Upstream `accounts_store` find/update of `AcquirerConfigBucket` on the business profile (`core:20-26,74-79,114-120,188-193`); C++ persists through `AppRepository` config records (`get_config`/`set_config` `:299-307`, `remember_config` `:374-391`, re-read `:135,249`) — a documented local substitute (`:24-27`), no processor/`accounts_store` or merchant key store. |
| Domain logic | PARTIAL | C++ now implements ID generation via `IdGenerator` pro_acq_ (`:124`), default-bucket set/clear (`:116-131,204-220`), ISO alpha2→numeric normalization (`:394-423`), per-network merge + duplicate detection (`:239-247,440-462`), and profile/bucket existence guards (`:108-114,189-202`); all via local config-record substitutes rather than upstream `core:17-101,105-215,219-312`. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/profile_acquirer` | `create_profile_acquirer` (v1/olap) | `handle_create_profile_acquirer` | `web::Json<ProfileAcquirerCreate>` (9 fields) vs typed DTO from JSON body | `ProfileAcquirerResponse` from `(id, profile_id, stored config, is_default)` vs re-read stored config with normalized country + `pro_acq_` IdGenerator id (`:124,133-136,394-423,464-489`) | `ApiKeyAuth`/`JWTAuthProfileFromRoute(ProfileAccountWrite)` vs `auth_rules` rule via `authorize_route` (`:88-90`) | `accounts_store.find_business_profile_by_profile_id` + `update_profile_by_profile_id` vs `AppRepository` config-record `get/set_config` (`:299-307,374-391`) | PARTIAL | `profile_acquirer.rs:13-50`; `core:12-102`; `app.rs:3528`; `profile_acquirer.cppm:82-143,186-197` |
| POST `/profile_acquirer/{profile_id}/{profile_acquirer_id}` | `profile_acquirer_update` (v1/olap) | `handle_profile_acquirer_update` | `web::Path<(ProfileId, ProfileAcquirerId)>` + `web::Json<ProfileAcquirerUpdate>` vs DTO from body + ids parsed from path segments | `ProfileAcquirerResponse` from re-read upserted config / `None` (is_default-only) via `build_response` (`:222-254,464-489`) | `ApiKeyAuth`/`JWTAuthProfileFromRoute(ProfileAccountWrite)` vs `auth_rules` rule via `authorize_route` (`:151-153`) | DB find/update `AcquirerConfigBucket` vs `AppRepository` config records + default key (`:196-250,299-307,374-391`) | PARTIAL | `profile_acquirer.rs:54-97`; `core:105-215`; `app.rs:3531-3532`; `profile_acquirer.cppm:145-261,198-208` |

## Not ported
- Business-profile lookup and `ProfileNotFound` error when the profile does not exist, plus merchant key store from the processor (`core:18-26,112-120`); C++ never loads a profile (`profile_acquirer.cppm:48-79,144-158`).
- `default_acquirer_config` bucket semantics: existing-default detection decides `is_default`, and create sets the pointer to the new id (`core:28-38,67-69`). C++ approximates existence via a cache key and only sets the key when default (`profile_acquirer.cppm:50-51,60-62,144-148,154-158`).
- ID generation via `generate_profile_acquirer_id_of_default_length()` producing `pro_acq_...` ids (`core:17`); C++ uses `"pa_" + std::hash<string>(body)` which is body-dependent and not a stable per-entity id (`profile_acquirer.cppm:53-54`).
- Country code normalization `Country::from_alpha2(code).to_numeric().to_string()` on both create and update (`core:47-51,268-272`); C++ echoes raw alpha2 string (`profile_acquirer.cppm:72-74,173-175`).
- `ProfileUpdate::AcquirerConfigBucketUpdate` persistence through `update_profile_by_profile_id` and re-read of the stored config used to build the response (`core:71-99,182-214`); C++ persists only the request body to cache and builds the response from the parsed request (`profile_acquirer.cppm:56-79,150-179`).
- Update-side existence guards: `acquirer_config_map` missing → `ProfileAcquirerNotFound`, and target `profile_acquirer_id` not in `configs` → `ProfileAcquirerNotFound` (`core:122-144`); no equivalent in C++ (`profile_acquirer.cppm:136-158`).
- `apply_default_bucket_change` transitions (true→set, false→clear `default_acquirer_config` → `None`); C++ clears the default via empty string rather than absence and does not track the bucket map (`core:151-156,219-236` vs `profile_acquirer.cppm:154-158`).
- `upsert_acquirer_config_in_bucket`: per-network slot merge using existing values as base (`request.or(base)`), `GenericDuplicateError` when an identical config exists with no default change, and replace-or-push into the bucket (`core:158-180,241-312`); absent in C++ (`profile_acquirer.cppm:165-179`).
- Response config selection: on create, find the stored config matching `network`; on update, return `None` when only `is_default` changed (`core:81-92,195-205`); C++ always echoes request fields including when `network` is absent (`profile_acquirer.cppm:64-78,169-177`).
- `#[serde(deny_unknown_fields)]` on `ProfileAcquirerUpdate` (`api_models/profile_acquirer.rs:154-156`); no corresponding unknown-field rejection in the typed DTO deserializer.
- Upstream scaffolding: `api::server_wrap` with `Flow::ProfileAcquirerCreate/Update`, `#[instrument]`, `api_locking::LockAction::NotApplicable`, and `#[cfg(all(feature = "olap", feature = "v1"))]` gating (`profile_acquirer.rs:11-22,47-49,52-67,94-96`).

## Implementation disposition

Dispositions re-checked against the current handler (2026-11-20). "SUBSTITUTED" = behavior implemented via a documented local mechanism, not upstream's exact dependency.

- Business-profile lookup / `ProfileNotFound` / merchant key store — SUBSTITUTED — `profile_exists` uses `repository().find_profile_by_string` / `list_merchant_accounts` / `list_profiles` (`profile_acquirer.cppm:325-338`) and emits `NOT_FOUND` (`:108-114,189-195`); no processor `merchant_key_store` (external dependency).
- `default_acquirer_config` bucket semantics — SUBSTITUTED — existing-default detection and `is_default` at `:116-119`, set pointer at `:129-131`, update-side set/clear at `:210-220`; pointer replaced by `profile_acquirer.default:<profile_id>` config record (`:309-312`).
- ID generation (`pro_acq_...`) — SUBSTITUTED — `m_context->ids().payment_id("pro_acq_")` at `:121-124` replaces upstream `generate_profile_acquirer_id_of_default_length()`; body-independent, not the upstream helper.
- Country code normalization — SUBSTITUTED — `normalize_country` ISO 3166-1 alpha2→numeric table with raw-alpha fallback at `:394-423`, applied on create (`:436`) and update (`:458-460`).
- `ProfileUpdate::AcquirerConfigBucketUpdate` persistence + re-read — SUBSTITUTED — `AppRepository` `get_config`/`set_config` at `:299-307`, `remember_config` at `:374-391`, re-read at `:135,249`; no `accounts_store` / business-profile `AcquirerConfigBucket` update.
- Update-side existence guards — IMPLEMENTED — `profile_exists` (`:189-195`) → `profile not found`, `bucket_exists` against the `profile_acquirer.ids:<profile_id>` index (`:196-202,340-356`) → `profile acquirer not found`.
- `apply_default_bucket_change` transitions — SUBSTITUTED — set/clear logic at `:210-220`; clear uses empty string (`:217`) rather than upstream `None`/pointer removal, via the config-record default key.
- `upsert_acquirer_config_in_bucket` (base merge + duplicate detection) — IMPLEMENTED — `merge_update` request-first fallback at `:239,440-462`; identical-config guard raises `DUPLICATE_REQUEST` (`:241-247`); replace/push via `remember_config` (`:248,374-391`).
- Response config selection (stored config / `None` for is_default-only) — IMPLEMENTED — `build_response` at `:464-489`; re-read config at `:135,249`, and `final_config` left nullopt when no `network` and no default change (`:222-254`).
- `deny_unknown_fields` on `ProfileAcquirerUpdate` — DEFERRED — handler deserializes via `serde::Ser::deserialize<Input>` (`:101,164`) with no unknown-field rejection; enforcement lives in the `app_dto` DTO deserializer (external dependency), not this handler.
- Upstream scaffolding (`server_wrap`/`Flow`/`#[instrument]`/locking/cfg gating) — DEFERRED — no such helpers in the handler; only `metrics().record_flow`/`counter` at `:138-139,256-257` stand in for instrumentation/flow tagging; gateway/locking/feature gating remain in route registration (external dependency).
