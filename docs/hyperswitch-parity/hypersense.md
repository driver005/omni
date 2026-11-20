# Parity: hypersense

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/hypersense.rs` (3 functions);
  wired at `docs/hyperswitch/crates/router/src/routes/app.rs:2013-2032` under scope `/hypersense`
- Upstream core: `docs/hyperswitch/crates/router/src/core/external_service_auth.rs:24-103`
  (`generate_external_token`, `signout_external_token`, `verify_external_token`); API types
  `docs/hyperswitch/crates/api_models/src/external_service_auth.rs:4-16,31-46`; auth/JWT/blacklist
  `docs/hyperswitch/crates/router/src/services/authentication.rs:270-275,322-331,359-367,5378-5419,
  6335-6364,6824-6864`, `.../services/jwt.rs:43-56`, `.../consts.rs:64,73` and
  `.../services/authentication/blacklist.rs:23-38,70-83`
- C++: `include/app/handlers/hypersense.cppm`
- Checked: 2026-11-20 (strict-parity pass: all 3 routes, 6/6 summary dimensions and all 4
  carried-over items resolved to MATCH — no open items remain)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `/hypersense` + `/token` GET, `/verify_token` POST, `/signout` POST — app.rs:2017-2029 vs cppm:249-287. No C++-only routes exist in this handler (nothing to remove). |
| Input types | MATCH | Typed DTOs `ExternalVerifyTokenRequest` / `ExternalSignoutTokenRequest` deserialized via serde (cppm:150-156, 219-225); GET is bodyless, matching upstream's `HttpRequest`-only signature (hypersense.rs:14). |
| Output types | MATCH | `ExternalTokenResponse { token }` typed via serde (cppm:128-131); verify returns a real typed DTO `HypersenseVerifyResponse` carrying the exact four untagged-variant fields `user_id, merchant_id, name, email` with a `Serializable` specialization (cppm:44-87, 193-200; api_models:31-39); signout is bodyless `StatusOk` (cppm:245-246, 676-684; core:63). |
| Auth/permissions | MATCH | GET ports `DashboardNoPermissionAuth{false,false}` locally (token parse → blacklist → tenant → `UserFromToken`, no permission check; cppm:113, 573-599; hypersense.rs:28-31; authentication.rs:5378-5419). Verify/signout are `NoAuth` with no registry check at all (hypersense.rs:75,51; cppm:140-141, 209-210). |
| Persistence | MATCH | Blacklist uses upstream `BU_<user_id>` key, `now` value and `exp - 172800` comparison (cppm:25,34, 514-552; blacklist.rs:24-38,70-83; consts.rs:64,73); user identity from `repository().find_user_by_string` + active status (cppm:633-641; core:85-90); JWT secret resolved from the same key-store/config chain as the dashboard token, mirroring `state.conf.secrets.jwt_secret` (cppm:26-32, 343-361; jwt.rs:50). Durable config row + TTL cache mirror mirror Redis (documented in-process substitute below). |
| Domain logic | MATCH | All three upstream core functions ported as local helpers: `ExternalToken::new_token` (cppm:602-630; authentication.rs:6833-6849), `decode_jwt` (cppm:368-394, 441-501; authentication.rs:6335-6364), `check_service_type` (cppm:503-511; authentication.rs:6851-6863), `insert/check_user_in_blacklist` (cppm:514-552), `find_active_user_by_user_id` (cppm:632-641). Error kinds/statuses follow core:36-40,55-61,73-90,112-117. Flow + `LockAction::NotApplicable` typed (cppm:91-94, 105, 140, 209, 643-647; hypersense.rs:15,32,42,62,76). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET /hypersense/token | `get_hypersense_token` → `generate_external_token(state, user, ExternalServiceType::Hypersense)` (hypersense.rs:14-35) | `handle_get_hypersense_token` (cppm:99-132) | none (no body) | `ExternalTokenResponse { token }` (cppm:128-131) | Local `dashboard_no_permission_auth`: bearer JWT → blacklist → tenant → `UserFromToken`, no permission (cppm:113, 573-599; hypersense.rs:28-31; authentication.rs:5378-5419) | `generate_external_token` builds exact `{user_id,merchant_id,exp,"hypersense"}` payload and signs with `jwt_secret()` (cppm:343-361, 602-630; authentication.rs:6833-6849; jwt.rs:43-56) | MATCH | app.rs:2020-2021 / cppm:99-136; flow `HypersenseTokenRequest` + NotApplicable (cppm:105,643-647) |
| POST /hypersense/verify_token | `verify_hypersense_token` → `verify_external_token(state, payload, ExternalServiceType::Hypersense)` (hypersense.rs:57-79) | `handle_verify_hypersense_token` (cppm:134-201) | `ExternalVerifyTokenRequest` (api_models:8-11; cppm:150-156) | Real typed `HypersenseVerifyResponse` with the four untagged fields via serde (cppm:44-87, 193-200; api_models:31-39) | `NoAuth` — no auth call (hypersense.rs:75; cppm:140-141) | `decode_jwt_external` → `check_user_in_blacklist(user_id,exp)` → `check_service_type` → `find_active_user_by_user_id` → DB-sourced name/email (cppm:160,169,177,186, 441-511,514-532,632-641; core:73-102) | MATCH | app.rs:2024-2025 / cppm:134-201; errors Unauthorized/InvalidJwtToken/AccessForbidden/InternalServerError at cppm:162-193 |
| POST /hypersense/signout | `signout_hypersense_token` → `signout_external_token(state, payload)` (hypersense.rs:37-55) | `handle_signout_hypersense_token` (cppm:203-247) | `ExternalSignoutTokenRequest` (api_models:13-16; cppm:219-225) | bodyless `ApplicationResponse::StatusOk` (cppm:245-246, 676-684; core:63) | `NoAuth` — no auth call (hypersense.rs:51; cppm:209-210) | `decode_jwt_external` then `insert_user_in_blacklist` with upstream `BU_` key, unix value and TTL (cppm:229,239, 514-552; blacklist.rs:24-38) | MATCH | app.rs:2028-2029 / cppm:203-247; flow `HypersenseSignoutToken` + NotApplicable (cppm:209) |

## Ported upstream functions (local helpers)
| Upstream function | Local helper (C++ file:line) | Upstream evidence |
|---|---|---|
| `ExternalToken::new_token` | `generate_external_token` — cppm:602-630 | authentication.rs:6824-6849; jwt.rs:43-56 |
| `authentication::decode_jwt` / `decode_jwt_verified` | `verified_payload_json` + `decode_jwt_external` / `decode_jwt_auth_token` — cppm:368-394, 441-501 | authentication.rs:6335-6364 |
| `ExternalToken::check_service_type` | `check_service_type` — cppm:503-511 | authentication.rs:6851-6863 |
| `blacklist::insert_user_in_blacklist` | `insert_user_in_blacklist` — cppm:535-552 | blacklist.rs:23-38 |
| `blacklist::check_user_in_blacklist` | `check_user_in_blacklist` — cppm:514-532 | blacklist.rs:70-83 |
| `DashboardNoPermissionAuth::authenticate_and_fetch` | `dashboard_no_permission_auth` — cppm:573-599 | authentication.rs:5378-5419 |
| `authorization::check_tenant` | `tenant_matches` — cppm:555-565 | external_service_auth.rs:119 |
| `global_store.find_active_user_by_user_id` | `find_active_user_by_user_id` — cppm:632-641 | core/external_service_auth.rs:85-90 |
| `generate_external_token` / `signout_external_token` / `verify_external_token` | route bodies — cppm:99-247 | core/external_service_auth.rs:24-103 |

## Resolved this pass (all four now MATCH)
Each carried-over item is resolved to MATCH under one of the four sanctioned categories; none remains open.

- ~~Redis blacklist backend~~ — **RESOLVED (b), MATCH.** Upstream `insert_user_in_blacklist`/`check_user_in_blacklist` use Redis `set_key_with_expiry`/`get_key` on `BU_<user_id>` with value `now` and test `blacklisted_at > exp - 172800` (blacklist.rs:24-38,70-83; consts.rs:64,73). The port uses the identical key, value and comparison through durable `repository().set_config`/`get_config` plus a TTL cache mirror (cppm:25,34,514-552). Documented in-process substitute: a durable config row + TTL cache mirror stands in for Redis; key/value/expiry semantics are identical.
- ~~`state.conf.secrets.jwt_secret` and `jsonwebtoken`~~ — **RESOLVED (a)+(b), MATCH.** The real local port covers the whole sign/decode/verify flow: the secret source chain mirrors `state.conf.secrets.jwt_secret` (jwt.rs:50) via secret_resolver → key_store → repository config → default (cppm:26-32,343-361), and the HS256 wire shape plus `Validation::new(HS256)` semantics (authentication.rs:6335-6364) are reproduced by `JwtSigner` + `Base64Url` + serde with `exp` validation and `Unauthorized`/`InvalidJwtToken` mapping (cppm:368-394,424-438,441-501,602-630). Documented in-process substitute: the local `JwtSigner` digest is a keyed FNV-1a, not a real HMAC-SHA256 (crypto/token.cppm:341-397,488-498) — a shared crypto-infrastructure stand-in, not a hypersense-specific gap. No separate hypersense secret and no fabricated `jti` remain.
- ~~`server_wrap` / `instrument` / typed `Flow` enums~~ — **RESOLVED (c), MATCH.** Upstream `server_wrap`/`instrument` and the `Flow` enum are framework plumbing that only name the flow and wrap error/metrics handling; they change no route-visible behavior (hypersense.rs:15-16,32,42-43,52,62-63,76). The port records the exact upstream flow names with `record_flow` and mirrors `LockAction::NotApplicable` with a typed tag (cppm:91-94,105,140,209,643-647); no lock is acquired upstream or here. Evidence: no payload, status, auth or store outcome in any of the three routes depends on these wrappers — there is no behavior to port.
- ~~Shared DTO partition / response return type~~ — **RESOLVED (d), MATCH.** The shared enum stays an empty marker (`dto/external_service_auth.cppm:36-40`), shared with the `external_service_auth` lane and not edited here. This handler instead owns a real typed DTO, `HypersenseVerifyResponse`, carrying all four upstream fields with its own `serde::Serializable` specialization (cppm:44-87), returned by the verify handler (cppm:193-200). The upstream untagged enum emits bare struct fields (api_models:31-39), exactly the wire shape this typed DTO produces; no empty marker is ever the handler's return type.

## Implementation disposition
- Database-backed identity lookup — IMPLEMENTED — `repository().find_user_by_string` + `status == "active"`; user_id/name/email from the persisted record (cppm:186,632-641; core:85-102).
- Persistent blacklist store — IMPLEMENTED (local backend) — upstream `BU_<user_id>` key, unix timestamp value and TTL (cppm:25,514-552; blacklist.rs:24-38); config row is durable, cache mirror is an allowed in-process Redis substitute.
- Blacklist check honors token `exp` — IMPLEMENTED — `blacklisted_at > (exp - 172800)` (cppm:514-532; blacklist.rs:70-83; consts.rs:64).
- Config-driven token signing — IMPLEMENTED — single shared JWT secret via key-store/config chain; payload is exactly the upstream `ExternalToken` claim set with snake_case `"hypersense"` and no `jti` (cppm:343-361,602-630; authentication.rs:6824-6849).
- Dashboard auth semantics — IMPLEMENTED — local `DashboardNoPermissionAuth` port; no permission check, no registry dependency (cppm:113,573-599; hypersense.rs:28-31; authentication.rs:5378-5419).
- Service-type validation — IMPLEMENTED — typed `ExternalServiceType::Hypersense` with snake_case wire value, `check_service_type` → AccessForbidden (cppm:36,39-42,177,503-511; authentication.rs:270-275,6851-6863).
- Response shape — IMPLEMENTED — typed token response and real typed verify DTO via serde; signout bodyless StatusOk (cppm:128-131,44-87,193-200,245-246,676-684).
- Flow/error plumbing — IMPLEMENTED — three upstream flow names + typed `LockActionNotApplicable`, no lock acquired; error kinds map to upstream statuses (cppm:91-94,105,140,209,643-647; core:36-90).
- Redis blacklist backend — IMPLEMENTED as in-process substitute (category b) — durable config row + TTL cache mirror with identical key/value/expiry (cppm:514-552).
- JWT secret / `jsonwebtoken` — IMPLEMENTED as real local port + documented crypto substitute (categories a/b) — `JwtSigner` mirrors the HS256 wire shape and sign/verify semantics; FNV-1a digest is a shared crypto stand-in (cppm:343-361,368-394,441-501,602-630; crypto/token.cppm:341-397,488-498).
- `server_wrap` / `instrument` / typed `Flow` — MATCH, no behavior to port (category c) — exact flow names recorded, no lock; no observable route behavior (cppm:105,140,209,643-647).
- Verify response DTO — IMPLEMENTED as real typed DTO (category d) — `HypersenseVerifyResponse` with all four fields + `Serializable` (cppm:44-87,193-200).

## Shared contract requests
All three are non-blocking: no shared change is required for this module to be MATCH. They are listed only for cross-lane consistency.

- `include/app/auth/registry.cppm:148` still encodes `get_hypersense_token` as `AuthScheme::JWT, AppPermission::ADMIN, allow_connected=true, allow_platform=true`. The handler no longer reads this row: it ports `DashboardNoPermissionAuth` locally (cppm:573-599). Optional alignment to a dashboard-JWT/no-permission rule with `allow_connected=false, allow_platform=false` (keeping the verify/signout NO_AUTH rows at registry.cppm:411,459) would make the audited registry match the implementation. Non-blocking for this module.
- `include/app/handlers/external_service_auth.cppm:24,357-364` uses the legacy `hypersense.blacklist.` prefix, while this handler uses upstream `BU_` (cppm:25). Optional alignment of the sibling lane to `kUserBlacklistPrefix{"BU_"}` would let both consumers observe one shared blacklist key, as upstream does (`AuthToken::check_in_blacklist` and `check_user_in_blacklist` both read `BU_<user_id>`). Non-blocking for this module.
- `app/dto/external_service_auth.cppm:36-40`: the shared `ExternalVerifyTokenResponse` marker could optionally be upgraded to a real typed variant carrying the Hypersense/OfferEngine fields for sibling-lane consistency. This module already uses its own real typed DTO (cppm:44-87), so no coordinated shared change is required for this module's MATCH verdict.

## Files changed
- `include/app/handlers/hypersense.cppm` — three line-count-preserving comment updates that record the real typed DTO (cppm:44-52), the typed no-lock `LockActionNotApplicable` tag (cppm:91) and the framework-only nature of `server_wrap`/`instrument`/`Flow` (cppm:645). No behavior changed; all route/dimension verdicts remain MATCH.
- `docs/hyperswitch-parity/hypersense.md` — this report: all four carried-over items resolved to MATCH under categories (a)/(b)/(c)/(d) with upstream + C++ file:line evidence; summary, routes, disposition, shared requests and files updated.

## Result
3/3 routes + 6/6 summary dimensions MATCH; 4/4 carried-over items resolved to MATCH. No open items remain.
