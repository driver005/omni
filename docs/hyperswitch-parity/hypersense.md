# Parity: hypersense

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/hypersense.rs` (3 functions);
  wired at `docs/hyperswitch/crates/router/src/routes/app.rs:2013-2032` under scope `/hypersense`
- Upstream core: `docs/hyperswitch/crates/router/src/core/external_service_auth.rs:24-103`
  (`generate_external_token`, `signout_external_token`, `verify_external_token`); API types
  `docs/hyperswitch/crates/api_models/src/external_service_auth.rs:4-16,31-46`; auth/JWT/blacklist
  `docs/hyperswitch/crates/router/src/services/authentication.rs:270-275,359-367,5378-5419,
  6335-6364,6824-6864` and `.../authentication/blacklist.rs:23-38,70-83`
- C++: `include/app/handlers/hypersense.cppm`
- Checked: 2026-11-20 (strict-parity pass; all named upstream functions ported as local helpers)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `/hypersense` + `/token` GET, `/verify_token` POST, `/signout` POST — app.rs:2017-2029 vs cppm:249-287. No C++-only routes exist in this handler (nothing to remove). |
| Input types | MATCH | Typed DTOs `ExternalVerifyTokenRequest` / `ExternalSignoutTokenRequest` deserialized via serde (cppm:152-154, 221-223); GET is bodyless, matching upstream's `HttpRequest`-only signature (hypersense.rs:14). |
| Output types | MATCH | `ExternalTokenResponse { token }` typed via serde (cppm:133-136); verify emits local serializable `HypersenseVerifyResponse` with the exact four untagged-variant fields `user_id, merchant_id, name, email` (cppm:53-69, 195-199; api_models:33-39); signout is bodyless `StatusOk` (cppm:246, 676-682; core:63). |
| Auth/permissions | MATCH | GET ports `DashboardNoPermissionAuth{false,false}` locally (token parse → blacklist → tenant → `UserFromToken`, no permission check; cppm:113, 573-603; hypersense.rs:28-31; authentication.rs:5378-5419). Verify/signout are `NoAuth` with no registry check at all (hypersense.rs:75,51; cppm:140-141, 209-210). |
| Persistence | MATCH | Blacklist uses upstream `BU_<user_id>` key, `now` value and `exp - 172800` comparison (cppm:24,34, 514-550; blacklist.rs:24-38,70-83; consts.rs:64,73); user identity from `repository().find_user_by_string` + active status (cppm:633-641; core:85-90); JWT secret resolved from the same key-store/config chain as the dashboard token, mirroring `state.conf.secrets.jwt_secret` (cppm:26-32, 343-358; jwt.rs:50). Durable config row + TTL cache mirror stand in for Redis (allowed local infra substitute). |
| Domain logic | MATCH | All three upstream core functions ported as local helpers: `ExternalToken::new_token` (cppm:606-631; authentication.rs:6833-6849), `decode_jwt` (cppm:368-388, 441-502; authentication.rs:6335-6364), `check_service_type` (cppm:504-512; authentication.rs:6851-6863), `insert/check_user_in_blacklist` (cppm:514-550), `find_active_user_by_user_id` (cppm:633-641). Error kinds/statuses follow core:36-40,55-61,73-90,112-117. Flow + `LockAction::NotApplicable` typed (cppm:91-94, 105, 140, 209, 643-647; hypersense.rs:15,32,42,62,76). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET /hypersense/token | `get_hypersense_token` → `generate_external_token(state, user, ExternalServiceType::Hypersense)` (hypersense.rs:14-35) | `handle_get_hypersense_token` (cppm:99-132) | none (no body) | `ExternalTokenResponse { token }` (cppm:133-136) | Local `dashboard_no_permission_auth`: bearer JWT → blacklist → tenant → `UserFromToken`, no permission (cppm:113, 573-603; hypersense.rs:28-31; authentication.rs:5378-5419) | `generate_external_token` builds exact `{user_id,merchant_id,exp,"hypersense"}` payload and signs with `jwt_secret()` (cppm:343-358, 606-631; authentication.rs:6833-6849; jwt.rs:43-56) | MATCH | app.rs:2020-2021 / cppm:99-136; flow `HypersenseTokenRequest` + NotApplicable (cppm:105,643-647) |
| POST /hypersense/verify_token | `verify_hypersense_token` → `verify_external_token(state, payload, ExternalServiceType::Hypersense)` (hypersense.rs:57-79) | `handle_verify_hypersense_token` (cppm:134-201) | `ExternalVerifyTokenRequest` (api_models:8-11; cppm:152-154) | `ExternalVerifyTokenResponse::Hypersense` four fields via serde (cppm:53-69, 195-199; api_models:33-39) | `NoAuth` — no auth call (hypersense.rs:75; cppm:140-141) | `decode_jwt_external` → `check_user_in_blacklist(user_id,exp)` → `check_service_type` → `find_active_user_by_user_id` → DB-sourced name/email (cppm:160,169,177,186, 441-512,514-533,633-641; core:73-102) | MATCH | app.rs:2024-2025 / cppm:134-201; errors Unauthorized/InvalidJwtToken/AccessForbidden/InternalServerError at cppm:162-193 |
| POST /hypersense/signout | `signout_hypersense_token` → `signout_external_token(state, payload)` (hypersense.rs:37-55) | `handle_signout_hypersense_token` (cppm:203-247) | `ExternalSignoutTokenRequest` (api_models:13-16; cppm:221-223) | bodyless `ApplicationResponse::StatusOk` (cppm:246, 676-682; core:63) | `NoAuth` — no auth call (hypersense.rs:51; cppm:209-210) | `decode_jwt_external` then `insert_user_in_blacklist` with upstream `BU_` key, unix value and TTL (cppm:229,239, 514-550; blacklist.rs:24-38) | MATCH | app.rs:2028-2029 / cppm:203-247; flow `HypersenseSignoutToken` + NotApplicable (cppm:209) |

## Ported upstream functions (local helpers)
| Upstream function | Local helper (C++ file:line) | Upstream evidence |
|---|---|---|
| `ExternalToken::new_token` | `generate_external_token` — cppm:606-631 | authentication.rs:6833-6849; jwt.rs:33-56 |
| `authentication::decode_jwt` | `verified_payload_json` + `decode_jwt_external` / `decode_jwt_auth_token` — cppm:368-388, 441-502 | authentication.rs:6335-6364 |
| `ExternalToken::check_service_type` | `check_service_type` — cppm:504-512 | authentication.rs:6851-6863 |
| `blacklist::insert_user_in_blacklist` | `insert_user_in_blacklist` — cppm:535-550 | blacklist.rs:23-38 |
| `blacklist::check_user_in_blacklist` | `check_user_in_blacklist` — cppm:514-533 | blacklist.rs:70-83 |
| `DashboardNoPermissionAuth::authenticate_and_fetch` | `dashboard_no_permission_auth` — cppm:573-603 | authentication.rs:5378-5419 |
| `authorization::check_tenant` | `tenant_matches` — cppm:555-570 | external_service_auth.rs:119 |
| `global_store.find_active_user_by_user_id` | `find_active_user_by_user_id` — cppm:633-641 | core/external_service_auth.rs:85-90 |
| `generate_external_token` / `signout_external_token` / `verify_external_token` | route bodies — cppm:99-247 | core/external_service_auth.rs:24-103 |

## Not ported (external-infra substitutions only)
- Redis blacklist backend: durable `repository().set_config("BU_<user>", now)` plus a TTL cache mirror replaces Redis `set_key_with_expiry`/`get_key` (blacklist.rs:28-35, 77-82). Key, value and expiry semantics are identical (cppm:24,34,514-550).
- `state.conf.secrets.jwt_secret` and `jsonwebtoken`: the shared key-store/config secret plus `app::crypto::JwtSigner` reproduce the HS256 wire shape and claim set; the signer's digest is a local keyed FNV-1a rather than a real HMAC-SHA256 (crypto/token.cppm:341-397,488-498). No separate hypersense secret or fabricated `jti` remains.
- `server_wrap`/`instrument`/typed `Flow` enums: represented by `record_flow("Hypersense*")` and a typed `LockActionNotApplicable` tag; no lock is acquired upstream or here (hypersense.rs:15,32,42,52,62,76; cppm:91-94,105,140,209,643-647).
- Shared DTO partition: `ExternalVerifyTokenResponse` is still an empty enum marker (`app/dto/external_service_auth.cppm:36-40`), so the four-field payload is emitted through the handler-local serde shape (cppm:53-69). This partition is shared with the `external_service_auth` lane and was not edited.

## Shared contract requests
- `include/app/auth/registry.cppm:148` still encodes `get_hypersense_token` as `AuthScheme::JWT, AppPermission::ADMIN, allow_connected=true, allow_platform=true`. The handler no longer reads this row: it ports `DashboardNoPermissionAuth` locally. Please align the registry row to a dashboard-JWT/no-permission rule with `allow_connected=false, allow_platform=false` (and keep the verify/signout NO_AUTH rows at registry.cppm:411,459) so the audited registry matches the implementation. Owned by the auth lane; not edited here.
- `include/app/handlers/external_service_auth.cppm:24,357-364` uses the legacy `hypersense.blacklist.` prefix, while this handler now uses upstream `BU_`. Align the sibling lane to `kUserBlacklistPrefix{"BU_"}` so both consumers observe one shared blacklist key, as upstream does (`AuthToken::check_in_blacklist` and `check_user_in_blacklist` both read `BU_<user_id>`).
- `app/dto/external_service_auth.cppm:36-40`: consider upgrading the `ExternalVerifyTokenResponse` marker to a real typed variant carrying the Hypersense/OfferEngine fields. It is shared with the `external_service_auth` lane, so both the DTO and both handlers need a coordinated change.

## Implementation disposition
- Database-backed identity lookup — IMPLEMENTED — `repository().find_user_by_string` + `status == "active"`; user_id/name/email from the persisted record (cppm:186, 633-641; core:85-102).
- Persistent blacklist store — IMPLEMENTED (local backend) — upstream `BU_<user_id>` key, unix timestamp value and TTL (cppm:24,514-550; blacklist.rs:24-38); config row is durable, cache mirror is an allowed in-process Redis substitute.
- Blacklist check honors token `exp` — IMPLEMENTED — `blacklisted_at > (exp - 172800)` (cppm:514-533; blacklist.rs:70-83; consts.rs:64).
- Config-driven token signing — IMPLEMENTED — single shared JWT secret via key-store/config chain; payload is exactly the upstream `ExternalToken` claim set with snake_case `"hypersense"` and no `jti` (cppm:343-358, 606-631; authentication.rs:6824-6849).
- Dashboard auth semantics — IMPLEMENTED — local `DashboardNoPermissionAuth` port; no permission check, no registry dependency (cppm:113, 573-603; hypersense.rs:28-31; authentication.rs:5378-5419).
- Service-type validation — IMPLEMENTED — typed `ExternalServiceType::Hypersense` with snake_case wire value, `check_service_type` → AccessForbidden (cppm:36,39-42,177,504-512; authentication.rs:270-275,6851-6863).
- Response shape — IMPLEMENTED — typed token/verify responses via serde; signout bodyless StatusOk (cppm:133-136, 53-69, 195-199, 246, 676-682).
- Flow/error plumbing — IMPLEMENTED — three upstream flow names + typed `LockActionNotApplicable`, no lock acquired; error kinds map to upstream statuses (cppm:91-94,105,140,209,643-647; core:36-90).

Files changed: `include/app/handlers/hypersense.cppm`, `docs/hyperswitch-parity/hypersense.md`.
Rows moved to MATCH: 3/3 routes and 6/6 summary dimensions (was 3 route PARTIAL; 3 summary PARTIAL).
Remaining: no route or summary PARTIAL remains; only the documented external-infra substitutes and the three shared-contract requests above.
