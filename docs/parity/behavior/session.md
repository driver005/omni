# Behavior report: session

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/session.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/auth/session/` (routes.cppm + create/, bulk_destroy/, session_verify_post/, session_verify_put/)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-19 (shared abstractions: `core_auth`, `blog_handlers_middleware`, `blog_handlers:outbox_shim`, `core::cache`-backed `ResourceStore`)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All four Ghost routes have a C++ counterpart at the same path+verb. The strict-parity
pass replaced plaintext password compares and fabricated codes with real Argon2id
password verification (`core_auth::PasswordHasher`/`PasswordHash`), RFC-6238 TOTP
generation/verification, Ghost-shaped error envelopes (including `Needs2FAError`
with `2FA_TOKEN_REQUIRED`/`2FA_NEW_DEVICE_DETECTED`), express-session-shaped cookies,
Ghost admin authorization on DELETE, and observable outbox mail side effects. DELETE
now matches Ghost. The other three remain PARTIAL only for registered-primitive-free
residuals: spam-prevention rate limiting/`Retry-After`, real SMTP delivery, geojs.io
IP geolocation, and bcrypt password hash verification (libsodium verifies Argon2id
PHC strings).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/session` | `add` (custom, returns Express middleware) | body `username`, `password`; optional `token` (body.token, read inside session service) | none (globalBlock + userLogin rate limits only) | 201 verified / 403 `Needs2FAError` / 401 `Access Denied.`; Set-Cookie `ghost-admin-api-session` | `User.getByEmail` → `User.check` → spam reset → `auth.session.createSession` → session-service regenerate/assign/verify/TOTP/mail | no cache; `rate_limits` table via `BackoffLimiter`; express-session store | empty `{}` body, status only | `auth/session/create/{handler,workflow/step/execute}.cppm` | Argon2id/plaintext-compatible `verify_password`, `last_seen`/`status` writes, session regeneration carrying prior verification+challenge, TOTP token verification, correct Needs2FA 403 envelope, Ghost-shaped cookie, mail outbox event; 201 or 403/401 | PARTIAL | Ghost `session.ts:23-87`, `middleware.ts:9-35`; C++ `create/.../execute.cppm:300-410` |
| DELETE `/session` | `delete` (custom) | none (Cookie header consumed) | `mw.authAdminApi` | 204, no body | `auth.session.logout` → `removeUserForSession` (clear verified/challenge/user_id) | no cache; express-session store mutation | empty, 204 | `auth/session/bulk_destroy/{handler,workflow/step/execute}.cppm` | GhostAuthorizer-based admin auth, in-place field clearing (not record destroy), no Set-Cookie, idempotent 204 | MATCH | Ghost `session.ts:88-96`, `middleware.ts:37-48`; C++ `bulk_destroy/handler.cppm`, `.../step/execute.cppm` |
| POST `/session/verify` | `sendVerification` (custom) | none (Cookie header consumed) | none (sendVerificationCode rate limit only) | 200, no body | `auth.session.sendAuthCode` → CSRF origin check → verificationEmail limiter → rotate challenge → TOTP generate → user lookup → GhostMailer send (+ geojs.io IP fetch) | no cache; `rate_limits` (`send_verification_code`, `send_verification_code_user`) | empty, 200 | `auth/session/session_verify_post/{handler,workflow/step/execute}.cppm` | session-id from cookie, CSRF origin binding check, challenge rotation + TOTP generation, user lookup, outbox mail event, 200; 400 `BadRequestError` on CSRF/missing user | PARTIAL | Ghost `session.ts:97-105`, `session-service.ts:424-484`; C++ `session_verify_post/.../execute.cppm` |
| PUT `/session/verify` | `verify` (custom) | body `token`; Cookie header | none (userVerification rate limit only) | 200 verified / 401 unverified | `auth.session.verifyAuthCode` → CSRF origin check → userVerificationChallenge limiter → `totp.verify` → `verifySession` (verified=true, invalidate challenge, clear email limit) | no cache; `rate_limits` (`user_verification`, `user_verification_challenge`) | empty, 200/401 | `auth/session/session_verify_put/{handler,workflow/step/execute}.cppm` | body token + cookie sid, CSRF check, 5-minute challenge validity, real TOTP verify (±10 step window), verified_user_id binding, challenge invalidation + email-limit reset outbox event, 200/401 | PARTIAL | Ghost `session.ts:106-114`, `session-service.ts:351-359,486-492`; C++ `session_verify_put/.../execute.cppm` |

## Route details

### POST /session — PARTIAL

**Ghost** (full path `/ghost/api/admin/session`; version variants via `versionRewrites`):
1. **Identity**: `router.post('/session', shared.middleware.rateLimiting.globalBlock, shared.middleware.rateLimiting.userLogin, http(api.session.add))` — `admin/routes.js:418-423`.
2. **Operation**: exported key `add`, custom action — no docName, validation, permissions or serializer pipeline (`api/endpoints/index.js:55-57`).
3. **Input**: `frame.data` = `{username, password}`; missing either → `UnauthorizedError('Access Denied.')` (`session.ts:24-32`). Optional `req.body.token` (`session-service.ts:282`), headers `origin`/`referrer`/`user-agent`, `req.ip`.
4. **Auth/permissions**: none; `globalBlock` + `userLogin` rate limits only. On success `userLogin().resetRequest` clears the IP counter (`session.ts:53-55`).
5. **Status + headers**: 201 via `res.sendStatus(201)` when verified; else `NoPermissionError` with `errorType:'Needs2FAError'`, code `2FA_TOKEN_REQUIRED`/`2FA_NEW_DEVICE_DETECTED`, 403 (`middleware.ts:17-31`); 401 `Access Denied.` for bad input. Cookie `ghost-admin-api-session` (`express-session.ts:19-32`).
6. **Call chain**: `User.getByEmail` (`session.ts:36`) → `hasLoggedIn()` → `User.check` (`session.ts:42-45`) → `createSession` (`middleware.ts:9-35`) → session-service regenerate/carry verification/assign/TOTP/mail. `PasswordResetRequiredError` → `generateResetToken` side effect (`session.ts:72-83`).
7. **Cache**: no HTTP cache (`Cache-Control: private`); rate-limit state in `rate_limits`; session in express-session store.
8. **Output**: status-only, empty body; cookie flags SSL/subdir-dependent.

**C++** (`include/blog/handlers/auth/session/`):
1. **Identity/route**: `Router{router, "/session"}.use(content_type_middleware).post(...)` → `session_create().handle(...)`; no auth or rate-limit middleware (`routes.cppm:19-55`).
2. **Operation**: `CreateHandler::handle` → `workflow.run("session_create", req)` → `ExecuteStep::invoke` (`create/.../execute.cppm`).
3. **Input**: `JsonBody{}.parse(in.body_as_string())` reads `username`, `password` and optional `token`; missing username/password → 401 `core::auth::ghost::unauthorized_error_json("Access Denied.")`.
4. **Password**: `support::verify_password` uses `core::auth::PasswordHash{stored}.verify(supplied)` for PHC-hashed values and direct compare only for fixture plaintext (`create/.../execute.cppm`). On success writes `last_seen` (ISO-8601) and `status:'active'`, mirroring `User.check`'s `updateLastSeen`/`save`.
5. **Session**: reads the prior session from the cookie, regenerates a fresh id via `resources().create("session", ...)`, carries verification only when `verified_user_id` matches this user, copies the auth challenge, and assigns origin/user-agent/ip. `body.token` is verified with `support::totp_verify` (RFC 6238, 6 digits, 60s step, ±10 window, 5-minute challenge validity).
6. **Status/headers**: 201 + `{}` + Ghost-shaped cookie when verified; 403 + `Needs2FAError` envelope (code from `require_email_mfa`) + cookie + `mail.signin_code` outbox event when unverified; 401 Ghost `UnauthorizedError` on missing fields or bad credentials. Cookie: `Max-Age`, `Path=<subdir>/ghost`, `HttpOnly`, `SameSite=None|Lax`, `Secure` on TLS.
7. **Cache**: session state lives in `ResourceStore` under the existing `"resources"` tag; no new namespace or TTL.
8. **Output**: status-only, `{}` on success; no `{data,meta}` envelope.

**Diff** — residual gaps (no registered primitive): spam-prevention `globalBlock`/`userLogin` rate limiting + `Retry-After`, bcrypt hash verification (libsodium supports Argon2id PHC, not `$2a$`), geojs.io IP geolocation used to enrich the mail, real SMTP delivery, password-reset token side effect on `PasswordResetRequiredError`, and express-session `Content-Type` parity. Divergence: plaintext fixture passwords still accepted for test compatibility.

### DELETE /session — MATCH

**Ghost**:
1. `router.delete('/session', mw.authAdminApi, http(api.session.delete))` (`admin/routes.js:424`).
2. `delete()` returns middleware calling `auth.session.logout` (`session.ts:88-96`).
3. `sessionService.removeUserForSession` clears `verified`/`verified_user_id` when `require_email_mfa`, invalidates the challenge, clears `user_id` (`session-service.ts:506-516`).
4. `res.sendStatus(204)`, empty body, no explicit `Set-Cookie` (`middleware.ts:37-48`).

**C++**:
1. `Router{router,"/session"}.delt(...)` → `session_bulk_destroy().handle(...)` (`routes.cppm:29-36`).
2. Handler enforces `mw.authAdminApi` equivalence: missing bearer token → 401 Ghost `UnauthorizedError`; otherwise `core::auth::ghost::GhostAuthorizer.authorize(principal, "session", "destroy")` with the `x-blog-*` principal (`bulk_destroy/handler.cppm`).
3. Step reads `ghost-admin-api-session` from the `Cookie` header and clears `verified`, `verified_user_id`, `user_id` and the auth challenge in place via `resources().edit("session", sid, ...)`; idempotent when the session is absent (`bulk_destroy/.../execute.cppm`).
4. Status 204, empty body, no `Set-Cookie` (`bulk_destroy/handler.cppm`).

**Diff** — none at the behavioral level. Admin auth is expressed through the project's
shared `GhostAuthorizer` (the standard equivalent of `mw.authAdminApi`); `require_email_mfa` is not a registered config primitive, so the strictest cleanup (clear the verified binding either way) is applied, which is observationally identical for logout.

### POST /session/verify — PARTIAL

**Ghost**:
1. `router.post('/session/verify', rateLimiting.sendVerificationCode, http(api.session.sendVerification))` (`admin/routes.js:425-429`); no auth.
2. `auth.session.sendAuthCode` → `sessionService.sendAuthCodeToUser` (`session-service.ts:424-484`): `getSession`, `cookieCsrfProtection`, `session.user_id` guard, verification-email limiter, challenge rotation, TOTP generate, user lookup, device details (+ geojs.io), `mailer.send`; 200 empty body.

**C++**:
1. Sub-route `Route{"/verify"}.post(...)` behind `content_type_middleware` (`routes.cppm:37-46`) → `SessionVerifyPostHandler`.
2. Step reads the session id from `Cookie`, validates the CSRF origin binding (`support::csrf_ok`) and requires a non-empty `user_id`, returning 400 `BadRequestError` otherwise.
3. `support::send_auth_code` rotates the challenge (16 random bytes → hex), generates a real TOTP code with `admin_session_secret`, looks up the user from the session, and emits a `mail.signin_code` outbox event; state persisted via `resources().edit` (`session_verify_post/.../execute.cppm`).
4. Handler writes 200 with an empty body; 400 carries the Ghost error envelope.

**Diff** — residual gaps: `sendVerificationCode`/`sendVerificationCodeUser` rate limits, real SMTP delivery (`core_adapters` is not a registered dep of `blog_handlers`), geojs.io geolocation, and config-bound admin origin comparison (the CSRF helper compares against the origin bound to the session instead of a configured `getAdminOrigin()`).

### PUT /session/verify — PARTIAL

**Ghost**:
1. `router.put('/session/verify', rateLimiting.userVerification, http(api.session.verify))` (`admin/routes.js:430-434`); no auth.
2. `auth.session.verifyAuthCode` → `verifyAuthCodeForUser` (`session-service.ts:351-359`): `getSession`, `cookieCsrfProtection`, `consumeAuthCodeAttempt`, `totp.verify(userId, token, admin_session_secret, challenge)` with 5-minute validity.
3. Verified → `verifySession` sets `verified=true`, `verified_user_id=user_id`, invalidates challenge, resets email limit; 200. Unverified → `res.sendStatus(401)` (empty body).

**C++**:
1. Sub-route `Route{"/verify"}.put(...)` (`routes.cppm:47-54`) → `SessionVerifyPutHandler`.
2. Step parses `token` from the body and the sid from `Cookie`; missing token/sid → 401 empty body (`sendStatus` parity).
3. CSRF origin binding checked (400 `BadRequestError` on mismatch); `support::has_valid_challenge` enforces the 5-minute window; `support::totp_verify` performs real RFC-6238 verification with the ±10-step window.
4. On success `support::verify_session` binds `verified_user_id`, invalidates the challenge, emits a `verification_email_limit.reset` outbox event and returns 200 empty; otherwise 401 empty (`session_verify_put/.../execute.cppm`).

**Diff** — residual gaps: `userVerification` rate limiting with `Retry-After` + `STAFF_VERIFICATION_ATTEMPTS_RATE_LIMITED`, config-bound `getAdminOrigin()` CSRF comparison, and real `otplib` secret handling. Divergences: none in the state machine (challenge validity, binding and invalidation all match).

## Module-level notes

- **Controller shape**: Ghost `session.ts` is exported raw (`api/endpoints/index.js:55-57`), bypassing `apiFramework.pipeline`; no docName/options/serializer. C++ handlers write status + empty body directly, matching.
- **Shared abstractions now used**: `core::auth::PasswordHash`/`GhostAuthorizer`/`unauthorized_error_json`, `blog::handlers::middleware::{principal_from_request,token,write_error,ErrorEnvelope}`, `blog::handlers::outbox::global_outbox()` (registered `outbox_shim` partition), `blog::api::{JsonBody,ApiError,parse_array}`, and `core::cache`-backed `ResourceStore`. A single exported `support` namespace hosted in the registered `session_create_step` partition provides cookie parsing, TOTP, session state and error envelopes to all four steps without adding a new partition or editing the shared BUILD.
- **Cookie**: generated with Ghost's flags (`Path=<subdir>/ghost`, `Max-Age`, `HttpOnly`, `SameSite=None|Lax`, `Secure` on TLS); logout emits no cookie (Ghost invalidates server-side only).
- **Password**: `verify_password` uses libsodium Argon2id PHC verification for hashed values; bcrypt ($2a$) fixtures and plaintext fixtures are documented residuals/compat paths.
- **2FA**: real TOTP (SHA-256/HMAC, step 60, 6 digits, ±10 window) generated and verified with the same secret/challenge; challenge is 16 random bytes hex-rotated, 5-minute validity, invalidated on success; mail is an observable outbox event.
- **Cache/side effects**: no HTTP cache; session + challenge state in `ResourceStore` under `"resources"`; mail and email-limit reset are outbox events.
- **Registering**: handlers exported through `all.cppm` and `base.cppm`; router via `routes.cppm:47` — unchanged.
- **EXTRA/MISSING**: none — exact path+verb match for all four Ghost routes.
