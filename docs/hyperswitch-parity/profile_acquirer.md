# Parity: profile_acquirer

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/profile_acquirer.rs` (`create_profile_acquirer` `:13-50`, `profile_acquirer_update` `:54-97`); registration `web::scope("/profile_acquirer")` with `POST ""` and `POST /{profile_id}/{profile_acquirer_id}` in `docs/hyperswitch/crates/router/src/routes/app.rs:3520-3534`
- Upstream core: `docs/hyperswitch/crates/router/src/core/profile_acquirer.rs` (`create_profile_acquirer` `:12-102`, `update_profile_acquirer_config` `:105-215`, helpers `apply_default_bucket_change` `:219-236` and `upsert_acquirer_config_in_bucket` `:241-312`)
- Upstream domain/models: `common_types/src/domain.rs:77-100` (`AcquirerConfig`), `:102-106` (`AcquirerConfigBucket`); `api_models/src/profile_acquirer.rs:5-32,108-138,154-174` (DTOs and `ProfileAcquirerResponse::from`)
- C++: `include/app/handlers/profile_acquirer.cppm` (all upstream core functions ported as local helpers)
- Checked: 2026-11-20 (handler rewritten this pass; every named upstream function ported)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Both expose `POST /profile_acquirer` and `POST /profile_acquirer/:profile_id/:profile_acquirer_id` (`app.rs:3528,3531-3532` vs `profile_acquirer.cppm:289-315`). No C++-only routes exist; nothing removed. |
| Input types | MATCH | Typed DTOs mirror upstream fields: create 9 fields, update 8 fields (including optional `network`/`is_default`) (`api_models/profile_acquirer.rs:5-32,156-174` vs `dto/profile_acquirer.cppm:70-100,137-164`, used at handler `:112,207`). |
| Output types | MATCH | Response built from the re-read stored `AcquirerConfig` via a field-for-field port of `ProfileAcquirerResponse::from` (`api_models:108-138` vs handler `build_response` `:772-802`, re-read `:161-171,274-280`); country encoded as unpadded numeric `u32::to_string()` (`transformers.rs:1544+` vs handler `normalize_country` `:555-624`). |
| Auth/permissions | MATCH | Local port of upstream's `auth_type` dual extractor (`routes:34-46,81-93`): Bearer → JWT + `PROFILE_WRITE` (≈`ProfileAccountWrite`) + `allow_connected(true)`/`allow_platform(false)`; otherwise `ApiKeyAuth` branch. Route-profile binding rejects an `x-profile-id` header that disagrees with the route profile and sets the principal to the route id (`handler authorize :333-378`), following the `verification.cppm:640-692` / `payout_link.cppm:354-374` convention. |
| Persistence | MATCH | Exact `AcquirerConfigBucket` state is reconstructed, mutated and re-read per request (`load_bucket` `:460-488`, `save_bucket` `:490-519`); AppRepository config records are the authorized in-process substitute for `accounts_store` / merchant key store (external infrastructure). No behavior-changing stand-in remains. |
| Domain logic | MATCH | All four upstream core functions ported verbatim as local helpers: `create` `:101-178`, `update` `:183-287`, `apply_default_bucket_change` `:642-660`, `upsert_acquirer_config_in_bucket` `:663-736`, plus ID generation `:530-545`, country normalization `:555-624`, existence guards `:428-458`, duplicate detection `:720-728`, response selection `:750-770`. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/profile_acquirer` | `create_profile_acquirer` (v1/olap) | `handle_create_profile_acquirer` (`:101-178`) | `web::Json<ProfileAcquirerCreate>` (9 fields) vs typed DTO from JSON body (`:112-117`) | `ProfileAcquirerResponse` from `(id, profile_id, stored config, is_default)`; id from `pro_acq_` generator (`:139`), config re-read from persisted bucket (`:161-171`), `build_response` (`:772-802`) | `auth_type(ApiKeyAuth, JWTAuthProfileFromRoute(ProfileAccountWrite))` (`:34-46`) vs dual-branch `authorize` with route-profile binding (`:333-378`, called `:122`) | `accounts_store` find/update `AcquirerConfigBucket` (`core:20-79`) vs `load_bucket`/`save_bucket` over AppRepository records (`:460-519`, called `:134,158`) | MATCH | `profile_acquirer.rs:13-50`; `core:12-102`; `app.rs:3528`; `profile_acquirer.cppm:101-178` |
| POST `/profile_acquirer/{profile_id}/{profile_acquirer_id}` | `profile_acquirer_update` (v1/olap) | `handle_profile_acquirer_update` (`:183-287`) | `web::Path<(ProfileId, ProfileAcquirerId)>` + `web::Json<ProfileAcquirerUpdate>` vs path ids (`:189-200`) + DTO (`:207-212`) | `ProfileAcquirerResponse` from re-read upserted config / `None` (is_default-only) via `build_response` (`:271-280`, `:772-802`) | `auth_type(...)` (`:81-93`) vs dual-branch `authorize` with route-profile binding (`:333-378`, called `:216`) | DB find/update `AcquirerConfigBucket` (`core:114-193`) vs `load_bucket`/`save_bucket` (`:460-519`, called `:228,269`) | MATCH | `profile_acquirer.rs:54-97`; `core:105-215`; `app.rs:3531-3532`; `profile_acquirer.cppm:183-287` |

## Not ported
No route-row behavior remains unported. The items below are external-infrastructure substitutes or shared-contract gaps, documented in Implementation disposition and `## Shared contract requests`; they are not behavior-changing stand-ins.

- `accounts_store` / `processor.get_key_store()` — no such objects exist in this C++ port. The exact `AcquirerConfigBucket` domain logic runs locally and is persisted through AppRepository config records (allowed in-process substitute).
- `server_wrap` / `#[instrument]` / `Flow` / `LockAction::NotApplicable` / `#[cfg(all(feature="olap",feature="v1"))]` — route-registration / framework concerns, outside the handler. No lock is acquired (equivalent to `NotApplicable`); upstream flow tags are recorded via `metrics().record_flow("ProfileAcquirerCreate"/"ProfileAcquirerUpdate")` (`:173,282`).
- `#[serde(deny_unknown_fields)]` on `ProfileAcquirerUpdate` and `CardNetwork`/`CountryAlpha2` enum validation — enforced by the shared `app_dto`/`core_serde` deserializer, not the handler (Shared contract requests SR-2).
- Exact upstream audit permission name `ProfileAccountWrite` and the coarse registry rows for these two operations — Shared contract request SR-1 (local mapping uses `PROFILE_WRITE`, as in `verification.cppm`).

## Implementation disposition

All dispositions re-checked against the rewritten handler (2026-11-20). "PORTED" = upstream logic reproduced locally with file:line evidence; "SUBSTITUTED" = external infrastructure replaced by an in-process mechanism with identical behavior.

- Business-profile lookup / `ProfileNotFound` — PORTED — `profile_exists` repository lookup (`:428-438`) and exact upstream message/name `profile_not_found` (`:805-812` ← `api_error_response.rs:474-476`), emitted at `:128-130,222-224`.
- `default_acquirer_config` bucket semantics — PORTED — `has_existing_default`/`is_default` (`:133-136` ← `core:28-38`), default pointer set on create (`:152-155` ← `core:67-69`) and set/clear via `apply_default_bucket_change` (`:642-660` ← `core:219-236`), cleared by resetting the optional to `None` (`:617`).
- ID generation (`pro_acq_...`) — PORTED — `generate_profile_acquirer_id_of_default_length` (`:530-545` ← `core:17`, `common_utils/lib.rs:680-684,699-702`, `consts.rs:7,10-15`): `pro_acq_` + 20 chars from the exact 62-char upstream alphabet. NanoID randomness is replaced by a deterministic PRNG seeded from `IdGenerator` (in-process substitute; shape/charset are exact).
- Country code normalization — PORTED — `normalize_country` (`:555-624`) embeds the full 249-entry alpha2→numeric table extracted verbatim from `common_enums/transformers.rs:277+` and `:1544+`, emitting **unpadded** numeric strings (e.g. AU→`"36"`). Applied on create (`:626-639`) and update (`:696`).
- `ProfileUpdate::AcquirerConfigBucketUpdate` persistence + re-read — PORTED (storage backend substituted) — `load_bucket`/`save_bucket` (`:460-519`) reconstruct the full bucket from config records, persist every mutation, and re-read before building the response (`:134,158,161,228,269,274` ← `core:71-99,182-214`).
- Update-side existence guards — PORTED — missing `acquirer_config_map` → `ProfileAcquirerNotFound` (`:227-232` ← `core:122-129`), target id absent from `configs` → `ProfileAcquirerNotFound` (`:237-241` ← `core:133-144`), with exact upstream message/name (`:813-822` ← `api_error_response.rs:477-479`).
- `apply_default_bucket_change` transitions — PORTED — true→set, false→clear to `None` (`:642-660` ← `core:219-236`).
- `upsert_acquirer_config_in_bucket` — PORTED — per-network `position`, base clone/default (`:677-694`), `request.or(base)` merge (`:696-711`), exact `GenericDuplicateError` message/name (`:718-727` ← `core:287-300`, `api_error_response.rs:687-689`), replace-or-push (`:729-734`).
- Response config selection — PORTED — `find_config_in_bucket` re-read on create (`:161-171`) and on update only when `network` was supplied; `None` for is_default-only (`:271-280`, builder `:772-802` ← `api_models:108-138`, `core:81-92,195-205`).
- `InvalidRequestData` message — PORTED — exact upstream text with HTTP 422 (`VALIDATION_FAILED` + name `InvalidRequestData`) at `:263-265,824-831` ← `core:174-179`, `api_error_response.rs:586-588`.
- Upstream scaffolding (`server_wrap`/`Flow`/`instrument`/locking/cfg gating) — DEFERRED — framework/route-registration concern; represented locally by flow metrics and no lock acquisition (see Not ported).

## Shared contract requests
- **SR-1 (auth registry / permission taxonomy):** `include/app/auth/registry.cppm` rows for `create_profile_acquirer` and `profile_acquirer_update` are coarse `API_KEY`/`ROUTING_WRITE`. The handler now ports upstream's `auth_type` dual branch locally with `PROFILE_WRITE` (the established local mapping for `ProfileAccountWrite`, cf. `verification.cppm:654-656`), so no shared edit is required for wire behavior; aligning the registry rows would be a cleanup only.
- **SR-2 (DTO serde validation):** `include/app/dto/profile_acquirer.cppm` + `core_serde` do not enforce `#[serde(deny_unknown_fields)]` on `ProfileAcquirerUpdate` nor `CardNetwork`/`CountryAlpha2` enum validation. Upstream rejects those at extraction (400) before core. Adding generic unknown-field rejection / enum validation to the shared deserializer would close this last framework-level gap; the handler cannot do it without re-parsing raw JSON.
- **SR-3 (domain model / key store):** no `common_types::domain::AcquirerConfig(Bucket)` or accounts-store/merchant-key-store facility exists in the C++ port. The handler carries local mirrors (`AcquirerConfig` `:31-56`, `AcquirerConfigBucket` `:89-93`) and persists via `AppRepository`; a shared domain type + accounts-store adapter would remove the substitute, but no behavior change is required.

## Report
- **Rows moved to MATCH:** 2/2 route rows (create + update). Summary dimensions changed to MATCH: Output types, Auth/permissions, Persistence, Domain logic (Route paths/verbs and Input types were already MATCH).
- **Remaining:** no route-row behavior gaps. Deferred framework scaffolding (server_wrap/instrument/locking/cfg gating) and three shared contract requests above (SR-1 cleanup, SR-2 serde validation, SR-3 shared domain type) — none changes emitted wire shape, auth decisions, error selection or persisted reads.
- **Shared requests:** SR-1, SR-2, SR-3 (above).
- **Files changed:** `include/app/handlers/profile_acquirer.cppm` (full rewrite with exact upstream logic), `docs/hyperswitch-parity/profile_acquirer.md` (this report). No other files touched; no C++-only routes removed (none existed).
