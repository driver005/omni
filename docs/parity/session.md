# Parity: session

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/session.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (Sessions block, lines 416-434)
- C++: `include/blog/handlers/auth/session/routes.cppm` (installed via `include/blog/handlers/routes.cppm:42,115`; handlers listed in `include/blog/handlers/all.cppm:177-180,523-526,772-775`)
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Ghost's session controller is not a data CRUD controller: each key returns an Express
middleware that performs real authentication work (password check, spam-prevention
reset, session cookie create/logout, verification-code send/verify). The strict-parity
pass rewired all four handlers to the shared abstractions: Argon2id password
verification (`core_auth`), Ghost-shaped error envelopes and admin authorization
(`blog_handlers_middleware`/`GhostAuthorizer`), RFC-6238 TOTP, express-session-shaped
cookies, and outbox-mail side effects (`blog_handlers:outbox_shim`). Path+verb,
operation, input, status/body and cross-request session state now agree. DELETE is a
full MATCH; POST/verify still have no registered primitive for rate limiting, real
SMTP delivery, geojs.io geolocation or bcrypt hash checking → PARTIAL.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/session` | `add` | `POST /session` | `session_create()` (`CreateHandler`) | Ghost: `frame.data.username`+`password`, required, optional `body.token` (session.ts:24-32, session-service.ts:282); no options/include/formats. C++: `JsonBody{}.parse(in.body_as_string())` reads username/password/token (`create/.../execute.cppm`) | Ghost: middleware sets `req.user`/`skipVerification`, `auth.session.createSession`, 201/403 `Needs2FAError`/401 (`session.ts:48-61`); no `{data,meta}`. C++: status+empty body, Ghost error envelopes, session cookie, outbox mail event | Ghost: `User.getByEmail`+`User.check`, reset-token path (session.ts:36-45,72-83). C++: `json_for("users", filter)`+`find_by_resource`, `verify_password` (Argon2id PHC or fixture plaintext), `last_seen`/`status` write, `resources().create("session",...)`, prior-verification/challenge carry-over | none both sides | Ghost: `rateLimiting.globalBlock`+`userLogin`, no admin auth. C++: `content_type_middleware`; no dead token check | PARTIAL | Ghost routes.js:418-422 / session.ts:23-87; C++ routes.cppm:19-28 / create/handler.cppm / create/.../execute.cppm |
| DELETE `/session` | `delete` | `DELETE /session` (`delt`) | `session_bulk_destroy()` (`BulkDestroyHandler`) | Ghost: no body/options (session.ts:88). C++: cookie sid only, no body parse | Ghost: middleware `auth.session.logout`, 204 empty (session.ts:89-94). C++: 204 empty, no Set-Cookie | Ghost: `auth.session.logout` → `removeUserForSession` field clearing (session.ts:94, session-service.ts:506-516). C++: `resources().edit("session", sid, ...)` clears verified/verified_user_id/user_id/challenge, idempotent | none both sides | Ghost: `mw.authAdminApi` (routes.js:424). C++: token check + `GhostAuthorizer.authorize(principal,"session","destroy")` with `x-blog-*` principal | MATCH | Ghost routes.js:424 / session.ts:88-96; C++ routes.cppm:29-36 / bulk_destroy/handler.cppm / bulk_destroy/.../execute.cppm |
| POST `/session/verify` | `sendVerification` | `POST /session/verify` | `session_session_verify_post()` (`SessionVerifyPostHandler`) | Ghost: no options; uses `req` (session.ts:97-104). C++: cookie sid only | Ghost: `auth.session.sendAuthCode`, 200 empty (session.ts:98-103). C++: 200 empty / 400 `BadRequestError` envelope | Ghost: `auth.session.sendAuthCode` → rotate challenge → TOTP → mail. C++: `csrf_ok` origin check, `send_auth_code` (random 16-byte challenge, TOTP generate, `mail.signin_code` outbox event) | none both sides | Ghost: `rateLimiting.sendVerificationCode`, no admin auth. C++: `content_type_middleware`; no dead token check | PARTIAL | Ghost routes.js:425-429 / session.ts:97-105; C++ routes.cppm:38-46 / session_verify_post/.../execute.cppm |
| PUT `/session/verify` | `verify` | `PUT /session/verify` | `session_session_verify_put()` (`SessionVerifyPutHandler`) | Ghost: `req.body.token`, cookie, origin (session-service.ts:356). C++: `JsonBody` `token` + cookie sid | Ghost: 200 verified / 401 unverified, empty body (middleware.ts:79-88). C++: 200/401 empty, 400 envelope on CSRF | Ghost: `auth.session.verifyAuthCode` → `totp.verify` → `verifySession`. C++: 5-minute `has_valid_challenge` + `totp_verify` (±10 window) → `verify_session` binds `verified_user_id`, invalidates challenge, emits `verification_email_limit.reset` | none both sides | Ghost: `rateLimiting.userVerification`, no admin auth. C++: `content_type_middleware`; no dead token check | PARTIAL | Ghost routes.js:430-434 / session.ts:106-114; C++ routes.cppm:47-54 / session_verify_put/.../execute.cppm |

## Route details

### POST `/session` — PARTIAL

- Route identity/verb: Ghost `router.post('/session', ...)` == C++ `Router{router, "/session"}.post(...)` (`routes.cppm:19,21`). Match.
- Operation: Ghost `api.session.add` (`session.ts:23`) ↔ `handler.session_create()` → `CreateHandler`.
- Input: Ghost requires `username`+`password` (401 `UnauthorizedError` otherwise); optional `body.token`. C++ `JsonBody{}.parse` reads all three; missing username/password → `core::auth::ghost::unauthorized_error_json("Access Denied.")`.
- Output: Ghost writes status directly; C++ writes status + empty `{}` body and never wraps in `{data,meta}`. 201 verified / 403 `Needs2FAError` envelope / 401 `UnauthorizedError`.
- DB call: Ghost `User.getByEmail`+`User.check`; C++ `json_for("users", {{"filter","email:"+username}})` + `find_by_resource`, `support::verify_password` (libsodium Argon2id PHC verify for hashed values, direct compare for plaintext fixtures) + `last_seen`/`status` write + `resources().create("session")`.
- Session regeneration: prior session read from cookie; verification carried only when `verified_user_id == user.id`; challenge copied; origin/user-agent/ip assigned; `body.token` verified with real TOTP when a valid challenge exists.
- Cookie: `support::session_cookie` emits `ghost-admin-api-session=<sid>; Max-Age=...; Path=<subdir>/ghost; HttpOnly; SameSite=None|Lax[; Secure]`.
- Cache: none on either side; session state in `ResourceStore` under `"resources"`.
- Auth: Ghost intentionally unauthenticated + rate limits; C++ unauthenticated. Rate limiting has no registered primitive → residual.

#### Fix (applied)

- The earlier opaque body read, plaintext-only compare and fabricated `challenge`/`first_login` logic were removed. The `ExecuteStep` now uses `JsonBody`, `support::verify_password`, real session regeneration, TOTP and Ghost error envelopes; the handler emits the Ghost-shaped cookie. `#### Fix` from the prior pass is closed; remaining gap is rate limiting / hashing format / mail / geo (no registered primitive). See `docs/parity/behavior/session.md` for the current Diff.

### DELETE `/session` — MATCH

- Route identity/verb: Ghost `router.delete('/session', ...)` == C++ `.delt(...)` (`routes.cppm:29`). Match.
- Output: 204 empty both sides; C++ emits no `Set-Cookie` (Ghost invalidates server-side only).
- DB call: Ghost `removeUserForSession` clears fields; C++ `resources().edit("session", sid, ...)` clears `verified`, `verified_user_id`, `user_id`, challenge in place; missing session → idempotent 204.
- Auth: Ghost `mw.authAdminApi`; C++ rejects empty bearer token with 401 Ghost `UnauthorizedError`, then `GhostAuthorizer.authorize(principal,"session","destroy")` on the `x-blog-*` principal. Equivalent within the shared abstraction set.
- Cache: none; context-local `ResourceStore` under `"resources"`.

#### Fix (applied)

- Removed the bare `token(req).empty()`-only check and the misleading expired-cookie write; added `GhostAuthorizer` admin authorization and in-place session field clearing. Verdict flipped from PARTIAL to MATCH: route, verb, no-input, 204 output, session teardown and admin auth all agree with Ghost.

### POST `/session/verify` — PARTIAL

- Route/verb/op: Ghost `router.post('/session/verify', ...)` == `Route{"/verify"}.post(...)` nested under `/session` (`routes.cppm:37-46`). Match.
- Input: cookie sid only; no body/options parsed.
- Output: 200 empty; 400 Ghost `BadRequestError` envelope on CSRF mismatch or missing `session.user_id`.
- DB call: `support::send_auth_code` rotates the challenge (`random_hex(16)`), generates a real TOTP code, resolves the user email from `session.user_id`, and emits a `mail.signin_code` outbox event; state persisted with `resources().edit("session", sid, ...)`.
- Auth: none (rate limit only) — no dead token check remains.

#### Fix (applied)

- The fabricated `std::hash(sid)%1e6` challenge was replaced by a rotated 16-byte challenge, real TOTP generation and an observable outbox mail event; CSRF origin binding is checked. Residual: rate limiting, real SMTP delivery, geojs.io geolocation and config-bound admin origin → PARTIAL.

### PUT `/session/verify` — PARTIAL

- Route/verb/op: Ghost `router.put('/session/verify', ...)` == `Route{"/verify"}.put(...)` (`routes.cppm:47-54`). Match.
- Input: `JsonBody` `token` + cookie sid; missing → 401 empty (`sendStatus` parity).
- Output: 200 verified / 401 unverified, empty body; 400 envelope on CSRF failure.
- DB call: `has_valid_challenge` (5-minute window), `totp_verify` (RFC 6238, ±10 steps) against the stored challenge; on success `verify_session` binds `verified_user_id`, invalidates the challenge, emits `verification_email_limit.reset` outbox event, 200.
- Auth: none (rate limit only) — no dead token check remains.

#### Fix (applied)

- The plain string comparison against a fabricated `challenge` was replaced by real TOTP verification with 5-minute validity and `verified_user_id` binding; the empty-id `edit("session","",...)` is gone. Residual: `userVerification` rate limiting with `Retry-After`/`STAFF_VERIFICATION_ATTEMPTS_RATE_LIMITED` and config-bound `getAdminOrigin()` → PARTIAL.

## Missing in C++

None. All four Ghost session routes have a C++ route+handler at the exact path and verb:

- POST `/session` → `session_create` (`routes.cppm:19-28`)
- DELETE `/session` → `session_bulk_destroy` (`routes.cppm:29-36`)
- POST `/session/verify` → `session_session_verify_post` (`routes.cppm:38-46`)
- PUT `/session/verify` → `session_session_verify_put` (`routes.cppm:47-54`)

## Extra in C++

None in the matched folder — `SessionRoutes::install` installs exactly the four routes above.
The `members_members_id_sessions_delete` route (`/members/:id/sessions`) is a different path
and belongs to the members controller, so it is not counted here.
