# Parity: external_service_auth

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/external_service_auth.rs` (1 route, `validate_token`); wired in `docs/hyperswitch/crates/router/src/routes/app.rs:2000-2011` under scope `/external-service`
- Upstream core: `docs/hyperswitch/crates/router/src/core/external_service_auth.rs` (196 lines; only `validate_token` at :107-137 is route-reachable — `generate_external_token`/`signout_external_token`/`verify_external_token` are not wired here); API types `docs/hyperswitch/crates/api_models/src/external_service_auth.rs`
- C++: include/app/handlers/external_service_auth.cppm
- Checked: 2026-11-19

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Upstream bare `POST /external-service/validate-token` now installed (`cppm:246-257`); legacy prefixed alias also registered (`cppm:233-244`). |
| Input types | PARTIAL | Wire normalizer `ValidateTokenWire` (`cppm:41-51,81-93`) + `parse_service` (`cppm:366-374`) accept upstream snake_case `offer_engine` and Display/kebab spellings, then map to typed DTO (`cppm:158-168`); token wrapped as `SecretString` (`cppm:170-171`). DTO enum itself still lacks string conversion — handler is a documented local substitute. |
| Output types | PARTIAL | Typed `OfferEngineVerifyResponse` emitted via serde (`cppm:61-77,95-109,221-226`) with `merchant_id, context, token, permissions` matching `api_models:40-45`; `context` hardcoded `MERCHANT` (`cppm:30,223`); token masked via `masked_as(ClientSecret)` (`cppm:224`); `Hypersense` variant unreachable from this route. |
| Auth/permissions | PARTIAL | Route rule now from `auth_rules().rule_for("validate_token")` with NO_AUTH passthrough (`cppm:127-137`); identity via JWT decode (`cppm:173-180,289-353`), blacklist (`cppm:182-187,355-364`), tenant config check (`cppm:189-195`), role lookup (`cppm:197-205,385-461`) — deterministic local substitutes, not upstream `NoAuth`+core plumbing. |
| Persistence | PARTIAL | JWT verified via `key_store()`/`repository().get_config` (`cppm:273-287`), blacklist durable marker in `repository()` + cache (`cppm:355-364`), tenant in `config_store()` (`cppm:190`), roles via `repository().list_user_roles/find_role_by_string` (`cppm:388-417`), merchant via `repository().find_merchant_account_by_string`/`superposition()` (`cppm:472-523`) — substitutes for upstream DB/JWT decode. |
| Domain logic | PARTIAL | `offer_engine_permissions` derives Offers permissions from role/config with `offers_` filter and forbidden on unresolved role (`cppm:385-461`); `resolve_offer_engine_merchant` implements credential-source application/merchant branches with forbidden on none (`cppm:472-523`). Fallback `{"offers_read","offers_write"}` still hardcoded for configured roles (`cppm:447`). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST /external-service/validate-token | validate_token | handle_validate_token (cppm:121) | ValidateTokenRequest via wire normalizer (cppm:41-51,158-168) | OfferEngineVerifyResponse typed serde + masked token (cppm:61-77,95-109,221-226) | NO_AUTH route rule + JWT/blacklist/tenant/role checks (cppm:127-205) | key_store + repository + config_store + superposition + cache (cppm:273-523) | PARTIAL | app.rs:2004-2009 / routes/external_service_auth.rs:11-27 / cppm:121-227,246-257; core/external_service_auth.rs:107-196 |

## Not ported
- HTTP-level auth semantics: upstream uses `authentication::NoAuth` with `api_locking::LockAction::NotApplicable` (`routes/external_service_auth.rs:23-24`); C++ enforces `AppPermission::ADMIN` on the incoming request via `TokenParser` (`cppm:32-39`). Different auth model.
- JWT identity decode: upstream `decode_jwt::<AuthToken>(&token, &state)` to get user/merchant/org/profile/tenant (`core/external_service_auth.rs:111-113`); C++ never decodes the token — uses it only as a cache-key fragment (`cppm:60-77`).
- Blacklist check: upstream `payload.check_in_blacklist(&state)` with `InvalidJwtToken` error (`core:115-117`); C++ checks cache key `hypersense.blacklist.<token>` and returns `UNAUTHORIZED` (`cppm:61-65`).
- Tenant validation: upstream `authorization::check_tenant(payload.tenant_id, &state.tenant.tenant_id)` (`core:119`); absent in C++.
- Role resolution: upstream `authorization::get_role_info(&state, &payload)` from DB (`core:121`); absent in C++.
- Offer Engine merchant resolution: upstream `resolve_offer_engine_merchant_id` via `Dimensions` (processor merchant id, org id, profile id) + `offer_engine::resolve_offer_engine_credential_source`, with `Application` vs `Merchant` config branches (latter reads key store + merchant account from `accounts_store`) and `AccessForbidden` when source is `None` (`core:139-187`); C++ synthesizes `merchant_<body_size>` or reads an unrelated cache key `hypersense.merchant.<token>` (`cppm:72-77`).
- Role-derived permissions: upstream `offer_engine_permissions` iterates `Permission::iter()`, filters `resource() == Resource::Offers` and `role_info.check_permission_exists`, serializing with `to_string()` (`core:189-196`); C++ hardcodes `["offers_read","offers_write"]` (`cppm:79`).
- `Secret<String>` masking on the token field (`api_models:27,43`); C++ uses plain `std::string` (`dto:54`).
- `ValidatingService` snake_case serde + `strum::Display` (`api_models:18-23`); C++ enum has no string conversion, so request deserialization of `"service": "offer_engine"` is not guaranteed to match (`dto:42-45,101-102`).
- `ExternalVerifyTokenResponse::Hypersense` variant and its `get_user_id` helper (`api_models:33-39,48-55`); not emitted anywhere — acceptable for this route since `ValidatingService` has only `OfferEngine`, but the DTO variant is only an empty enum marker in C++ (`dto:36-40`).
- Route prefix fidelity: C++ prepends `/external_service_auth` (`cppm:95`); upstream exposes the handler at `/external-service/validate-token` with no such prefix (`app.rs:2004-2009`).
- `server_wrap` / `instrument` flow plumbing with `Flow::ExternalServiceValidateToken` (`routes/external_service_auth.rs:16-26`); C++ has no equivalent flow instrumentation.
- Core functions not wired to any route in this domain: `generate_external_token`, `signout_external_token`, `verify_external_token` (`core/external_service_auth.rs:24-103`) — out of scope for the single-route upstream router but noted for completeness.

## Implementation disposition
- HTTP-level auth semantics — SUBSTITUTED — now `auth_rules().rule_for("validate_token")` + `authorize_route` with NO_AUTH passthrough (`cppm:127-137`); no `AppPermission::ADMIN` TokenParser.
- JWT identity decode — IMPLEMENTED — `decode_dashboard_claims` verifies JwtSigner signature, decodes Base64Url payload, reads user/merchant/org/profile/tenant/role/external_service + exp (`cppm:173-180,289-353`); signing secret from key_store then repository config then local default (`cppm:273-287`).
- Blacklist check — SUBSTITUTED — `is_blacklisted` checks repository durable marker + cache keyed on decoded user_id, returns UNAUTHORIZED (`cppm:182-187,355-364`).
- Tenant validation — IMPLEMENTED — config_store `tenant.tenant_id` compared to decoded `claims->tenant_id`, mismatch → FORBIDDEN (`cppm:189-195`).
- Role resolution — SUBSTITUTED — `offer_engine_permissions` loads active role from `repository().list_user_roles()` / `find_role_by_string()` (`cppm:388-417`).
- Offer Engine merchant resolution — SUBSTITUTED — `resolve_offer_engine_merchant` uses `offer_engine.credential_source` with application (superposition/config) and merchant (repository account + config override) branches, empty result → FORBIDDEN (`cppm:207-214,472-523`).
- Role-derived permissions — SUBSTITUTED — `offer_engine.role_permissions.<role_id>` parsed with `offers_` filter + dedup; unresolved role → FORBIDDEN; fallback `{"offers_read","offers_write"}` when role config absent (`cppm:419-461,447`).
- `Secret<String>` masking — IMPLEMENTED — token wrapped in `app::crypto::SecretString`, emitted via `masked_as(ClientSecret)` (`cppm:170-171,224`).
- ValidatingService snake_case serde + strum Display — SUBSTITUTED — `ValidateTokenWire` keeps `service` as text; `parse_service` accepts `offer_engine` / `OfferEngine` / `offer-engine` then maps to typed enum (`cppm:41-51,158-163,366-374`).
- `ExternalVerifyTokenResponse::Hypersense` variant — DEFERRED — OfferEngine arm now fully typed and emitted (`cppm:61-77,95-109,221-226`); Hypersense stays unreachable for this route since `ValidatingService` has only OfferEngine (`api_models:18-23`).
- Route prefix fidelity — IMPLEMENTED — bare upstream path mounted at `cppm:246-257`; legacy prefixed alias retained at `cppm:233-244`.
- server_wrap / instrument flow plumbing — SUBSTITUTED — `metrics().record_flow("ExternalServiceValidateToken")` + counter increment (`cppm:216-219`); no true server_wrap/instrument wrapper.
- Core functions not wired to any route — DEFERRED — generate/signout/verify_external_token still unwired upstream (`core:24-103`), out of scope for this single-route router; signout durable blacklist marker is consumed at `cppm:355-364`.
