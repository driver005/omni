# Behavior report: authentication

- Ghost controller: `ghost/core/server/api/endpoints/authentication.js`
- Ghost route table: `ghost/core/server/web/api/endpoints/admin/routes.js` (Authentication block, lines 439–457)
- Ghost output serializer: `ghost/core/server/api/endpoints/utils/serializers/output/authentication.js` (per-op response envelopes)
- C++ folders: `include/blog/handlers/auth/authentication/` (routes.cppm + 8 handler/flow/step triplets)
- Analyzed: 2026-11-19; strict-parity rewrite applied same date
- Registration: `include/blog/handlers/BUILD.bazel:684-707` (steps/flows/handlers), `include/blog/handlers/BUILD.bazel:970` (routes). No new partitions, no shared BUILD / all.cppm / base.cppm / central-route edits.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 8 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Eight Ghost routes bind this controller; all eight have a C++ route at the same path + verb.
Every C++ step now reproduces the exact Ghost wire contract for its operation: input envelope,
validators, setup guard, permission/auth posture, status, cache headers, context-store mutations,
outbox side effects, and the **per-op response envelope from the Ghost `authentication` output
serializer**. No C++ route is an EXTRA. Cross-layer Ghost internals with no primitive in this
codebase (mail transport, HMAC token signing, Express-session rotation, rate limiting, bcrypt)
are documented as residual gaps below, as in `behavior/previews.md`.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/authentication/password_reset` | `generateResetToken` | `data.password_reset[0].email`, `options:['email']` | public; `permissions:true`; globalReset+userReset rate limits | 200; no `X-Cache-Invalidate` | `assertSetupCompleted(true)` → `generateToken` (db_hash, getByEmail, resetToken.generateHash +1d) → `sendResetNotification` (mail) | none | `{"password_reset":[{"message":"Check your email for further instructions."}]}` | `authentication_password_reset_post/` | `core::query` option parse + `data.password_reset[0]` unwrap; 400 BadRequestError invalid email; setup guard; `core::auth::ghost` permissions; user lookup with 404 suspended-user 403; expiring token in context store + mail outbox event; exact serializer envelope | MATCH | G: authentication.js:140–157, routes.js:440–445, output/authentication.js#generateResetToken; C++: step:16-77, handler:22-29 |
| PUT `/authentication/password_reset` | `resetPassword` | `data.password_reset[0].{token,oldPassword,newPassword,ne2Password}`, `options:['ip']` | public + `auth.session.initSession`; globalBlock | 200; no `X-Cache-Invalidate` | `assertSetupCompleted(true)` → `extractTokenParts` → `doReset` (Knex tx, compare, changePassword, status active) → session rotate → spam reset → `{}` | none | `{"password_reset":[{"message":"Password updated"}]}` | `authentication_password_reset_put/` | password-match validator (422 exact message); setup guard; base64 + Ghost/canonical token extract with expired/used/corrupted branches; user 404 + suspended 403; password+status mutation, single-use token destroy, session/spam outbox events; exact serializer envelope | MATCH | G: authentication.js:159–205, passwordreset.ts:106–209, output/authentication.js#resetPassword; C++: step:16-92 |
| POST `/authentication/invitation` | `acceptInvitation` | `data.invitation[0].{token,name,password}` | public; `permissions:false` | 200; no `X-Cache-Invalidate` | `assertSetupCompleted(true)` → `invites.service.accept` (tx, find invite by token, expiry, User.add) | none | `{"invitation":[{"message":"Invitation accepted."}]}` | `authentication_invitation_post/` | singular `data.invitation[0]` unwrap with token/password/name validators; setup guard; base64 token; find `status:'sent'` invite, expiry 404, duplicate-email 422, invite destroy + user creation with invite role in context store; exact serializer envelope | MATCH | G: authentication.js:207–219, accept.ts:49–98, output/authentication.js#acceptInvitation; C++: step:14-72 |
| GET `/authentication/invitation` | `isInvitation` | query `email` (`data:['email']`) | public; `permissions:false` | 200; no `X-Cache-Invalidate` | `assertSetupCompleted(true)` → `models.Invite.findOne({email,status:'sent'})` → serializer `valid` flag | none | `{"invitation":[{"valid":true|false}]}` | `authentication_invitation_get/` | `core::query` email option parse + strict 400 email validator; setup guard; store `status:'sent'` lookup; exact `valid` envelope on both hit and miss (Ghost returns 200, not 404) | MATCH | G: authentication.js:221–235, input/invitations.js:34-44, output/authentication.js#isInvitation; C++: step:14-45 |
| POST `/authentication/setup` | `setup` | `data.setup[0].{name,email,password,blogTitle,theme,accentColor,description}` | public; `permissions:false` | 201; `X-Cache-Invalidate: /*` | `assertSetupCompleted(false)` → `setupUser` → `doFixtures` → `doProductAndNewsletter` → `installTheme` → `doSettings` → `sendWelcomeEmail` | header | `{"users":[<mapped user>]}` | `authentication_setup_post/` | setup-state guard (`'Setup has already been completed.'`); owner create/update; fixture posts + default tier/newsletter rename in store; theme/welcome-mail outbox events; title/description/accent_color settings; cleaned user model; 201 + `/*` cache header | MATCH | G: authentication.js:31–86, setup.js:65–244, output/authentication.js#setup; C++: step:15-90, handler:26-27 |
| PUT `/authentication/setup` | `updateSetup` | `data.setup[0].{name,email,password,blogTitle}` | `mw.authAdminApi` + owner-only permission fn | 200; `X-Cache-Invalidate: /*` | `assertSetupCompleted(true)` → `setupUser` → `doSettings` | header | `{"users":[<mapped user>]}` | `authentication_setup_put/` | per-verb auth guard in `routes.cppm` + `core::auth::ghost` owner check (`'You are not the site owner.'`); setup guard; owner row + settings mutation; cleaned user model; 200 + `/*` cache header | MATCH | G: authentication.js:88–115, input/setup.ts:13-18, output/authentication.js#updateSetup; C++: step:107-166, routes.cppm:20-33,106-134 |
| GET `/authentication/setup` | `isSetup` | none | public; `permissions:false` | 200; no `X-Cache-Invalidate` | `checkIsSetup()` → `User.isSetup()` → `{status:true}` or `{status:false,title,name,email}` from config | none | `{"setup":[{status:true}|{status:false,title,name,email}]}` | `authentication_setup_get/` | `User.isSetup()`-equivalent store check (active owner); false branch reads context-local `config` resource then settings; exact serializer envelope | MATCH | G: authentication.js:117–138, setup.js:32–34, output/authentication.js#isSetup; C++: step:14-33 |
| POST `/authentication/reset` | `reset` | options only (`context.user` actor) | `mw.authAdminApi` (staff-token block) + `permissions:true` | 200; no `X-Cache-Invalidate` | `resetAuthentication` (rotate keys, lock users, audit, clear keys, delete sessions, reschedule) → `destroyRequestSession` | none | `{"security_action":[{"action":"reset_authentication","api_keys_rotated":N,"users_locked":M}]}` | `authentication_reset_post/` | per-verb auth guard in `routes.cppm`; staff/integration token 403 envelopes; `core::auth::ghost` permissions; real key rotation + user lock counts, audit `Action.add` row, session deletion, internal-key clear + scheduler/cookie outbox events; exact serializer envelope | MATCH | G: authentication.js:237–258, reset-authentication.ts:32–77, output/authentication.js#reset; C++: step:16-84, routes.cppm:20-33,90-104 |

## Route details

### POST `/authentication/password_reset` — MATCH

**Ghost**

1. Identity: `POST /authentication/password_reset`; router-wide `apiMw.cors` (`routes.js:17`), then `shared.middleware.rateLimiting.globalReset` + `userReset` (`routes.js:442–443`), then `http(api.authentication.generateResetToken)`. No auth middleware. `userReset`/`globalReset` emit `TooManyRequestsError` (429) on block (`spam-prevention.ts:254–266`).
2. Operation: key `generateResetToken`, custom op, `docName:'authentication'`, validation `docName:'password_reset'`.
3. Input: `options:['email']` (`authentication.js:148`); body `data.password_reset[0].email`; validator `generateResetToken` requires a valid email string else `BadRequestError('The server did not receive a valid email')` (`input/password_reset.js:30–39`).
4. Auth/permissions: `permissions:true` (`authentication.js:147`); no `authAdminApi` on route.
5. Status/headers: no explicit `statusCode` → 200; `headers.cacheInvalidate:false` (`authentication.js:141–143`) ⇒ no `X-Cache-Invalidate` header (`@tryghost/api-framework/src/headers.ts` only emits the header when the flag is truthy; `true` emits `/*`).
6. Call chain: `assertSetupCompleted(true)()` (`setup.js:42–63`, throws `NoPermissionError('Setup must be completed before making this request.')` if not set up) → `auth.passwordreset.generateToken(email, api.settings)` → `settingsAPI.read({key:'db_hash'})`, `models.User.getByEmail` → `NotFoundError('User not found.')` (`passwordreset.ts:74–79`), `NoPermissionError('This account has been suspended.')` for inactive users (`:85–87`), `resetToken.generateHash` with `expires=now+1 day` → `sendResetNotification` builds the reset link and calls `mailAPI.send` (`:211–242`).
7. Cache: `cacheInvalidate:false`; no cache service call.
8. Output: the `authentication` output serializer's `generateResetToken` (`output/authentication.js`) replaces the controller return with `{"password_reset":[{"message":"Check your email for further instructions."}]}` (200). A mail transport failure surfaces as a Ghost `EmailError` 500 before serialization.

**C++**

1. Route `POST /authentication/password_reset` with `content_type_middleware`, no auth middleware (`routes.cppm:70-88`).
2. Input: `core::query::parse(path, {"email"})` for the allowlisted Ghost option; body `data.password_reset[0]` unwrapped from the JSON body. Missing/invalid email → 400 Ghost `BadRequestError` envelope with the exact Ghost message (`step:22-24`).
3. `assertSetupCompleted(true)` is implemented against the context store's active-owner check (`step:26-27`). `permissions:true` runs through `core::auth::ghost::GhostAuthorizer` under the trusted-api principal (`step:29-34`); an explicit role-scoped principal is still authorizable.
4. User lookup via `store.json_for("users", {{"filter","email:"+email}})`; empty → 404 `NotFoundError('User not found.')`; `status=='inactive'` → 403 `NoPermissionError('This account has been suspended.')` (`step:36-42`). Locked users remain recoverable, as in Ghost.
5. `generateToken`: reads `db_hash` from the context store, derives an expiring token (`now + 1 day`) bound to the user id/email/db hash, and records the issued token in `password_reset_tokens` (`step:44-56`). `sendResetNotification` is recorded as a `mail.reset_password` outbox event (`:58-62`).
6. Output: the exact serializer envelope `{"password_reset":[{"message":"Check your email for further instructions."}]}`, status 200 (`step:64-67`). Handler emits no cache header (`handler.cppm:26-28`).

**Diff**

- Residual gaps (no primitive in this codebase): `@tryghost/security` HMAC token signing (the 24h/expiry semantics are reproduced), SMTP transport (no `EmailError` 500 branch), express-session, and rate limiting (429). Recorded on the shared outbox where practical.
- Divergences: none at the endpoint wire contract.

### PUT `/authentication/password_reset` — MATCH

**Ghost**

1. Identity: `PUT /authentication/password_reset`; `rateLimiting.globalBlock` (`routes.js:448`), `auth.session.initSession` (`:449`), then controller.
2. Validation `password_reset`: `data:{newPassword:{required:true},ne2Password:{required:true}}` + `newPassword === ne2Password` else `ValidationError('Your new passwords do not match')` (`input/password_reset.js:16–27`). `options:['ip']`.
3. `assertSetupCompleted(true)` → `extractTokenParts` base64-decodes and extracts `expires`/`email` (`UnauthorizedError` on corrupted token, `passwordreset.ts:106–128`) → `doReset` in a Knex transaction: `db_hash`, `User.getByEmail` (404), `forUpdate` row locks, `resetToken.compare` (`BadRequestError` expired / already-used), inactive → 403, `User.changePassword` + status active (`:140–202`) → session rotation → spam reset → `{}`.
4. Output serializer `resetPassword` replaces `{}` with `{"password_reset":[{"message":"Password updated"}]}` (optionally `emailVerificationToken`).

**C++**

1. Public route, no auth middleware (`routes.cppm:70-88`); `core::query::parse(path, {"ip"})` captures the `options:['ip']` value (fallback `X-Forwarded-For`).
2. `data.password_reset[0]` unwrap; required + equal `newPassword`/`ne2Password` else 422 `ValidationError('Your new passwords do not match')` (`step:22-29`); setup guard (`:31-32`).
3. Token extraction accepts both Ghost's base64 `expires|email|hash` wire form and this handler's issued `id:expires:email:db_hash` form; malformed → 401 `UnauthorizedError` with Ghost's context/help (`step:34-50`). Unknown local token → 400 already-used; expired → 400 expired (`step:51-66`).
4. User lookup 404 / inactive 403, then password + `status:'active'` mutation on the user row, single-use token destroy, and `session.rotated` / `spam.user_login_reset` outbox events (`step:67-86`).
5. Output: exact serializer envelope `{"password_reset":[{"message":"Password updated"}]}`, 200, no cache header (`step:88-90`, `handler.cppm:26-28`).

**Diff**

- Residual gaps: bcrypt hashing / HMAC `resetToken.compare` (expiry, single-use, and corrupted-token branches are reproduced via the stored issuing token and decoded token parts), Knex row locks, session regeneration at the HTTP layer, spam-prevention counter, rate limiting.
- Divergences: none at the endpoint wire contract.

### POST `/authentication/invitation` — MATCH

**Ghost**

1. Public route (`routes.js:452`), `permissions:false`, `cacheInvalidate:false`.
2. Validation `invitations`: `data.invitation[0].{token,password,name}` all required with exact messages `'No token provided.'`, `'No password provided.'`, `'No name provided.'` (`input/invitations.js:14–32`).
3. `assertSetupCompleted(true)` → `invites.service.accept(frame.data)` → `accept.ts:49–98`: base64-decode token, `Invite.findOne({token,status:'sent'})` (404 `'Invite not found.'`), `expires < Date.now()` (404 `'Invite is expired.'`), duplicate email (422 `'Could not create an account, email is already in use.'` + context/help), `invite.destroy`, `User.add({email,name,password,roles:[roleId]})`.
4. Serializer `acceptInvitation` → `{"invitation":[{"message":"Invitation accepted."}]}`.

**C++**

1. Public route (`routes.cppm:50-68`); singular `data.invitation[0]` unwrapped with the three exact validators (422) (`step:15-19`).
2. Setup guard (`:21-22`); base64 token decode; `status:'sent'` invite lookup in the context store, expiry 404, duplicate-email 422 with Ghost context/help (`step:24-41`).
3. Transactional effect reproduced as invite destroy + user creation (with invite `role_id`) via the context store (`step:43-51`).
4. Output: exact serializer envelope, 200, no cache header (`step:53-55`).

**Diff**

- Residual gaps: none at the endpoint contract; the base64/HMAC invite token and DB row locks are represented by the store's issued invite row.
- Divergences: none. The former MISMATCH (creating a fresh `invitations` row) is resolved.

### GET `/authentication/invitation` — MATCH

**Ghost**

1. Public route (`routes.js:453`); `data:['email']`, validation `invitations` → invalid email string is `BadRequestError('The server did not receive a valid email')` (400, `input/invitations.js:34–44`).
2. `assertSetupCompleted(true)` → `models.Invite.findOne({email,status:'sent'}, frame.options)`. A miss returns `null` (Bookshelf `findOne` does not throw), which the serializer maps to `valid:false` — the route stays 200.
3. Serializer `isInvitation` → `{"invitation":[{"valid":!!data}]}`.

**C++**

1. Public route (`routes.cppm:50-68`); `core::query::parse(path, {"email"})` + strict email validator → 400 `BadRequestError` with the exact message (`step:14-20`); setup guard (`:22-23`).
2. `store.json_for("invites", {{"filter","email:"+email}})` with an in-step `status=='sent'` + email check (`step:25-33`).
3. Output: exact `valid:true`/`valid:false` envelope, 200, no cache header (`step:35-38`). The former fabricated 404 / raw-record JSON is removed.

**Diff**

- Residual gaps: none at the endpoint contract.
- Divergences: none.

### POST `/authentication/setup` — MATCH

**Ghost**

1. Public route (`routes.js:454`); `statusCode:201`, `cacheInvalidate:true` ⇒ `X-Cache-Invalidate: /*`.
2. `validation:{docName:'setup'}` (no `setup` validator method exists); body `data.setup[0]` with `status:'active'` forced.
3. `assertSetupCompleted(false)` → `NoPermissionError('Setup has already been completed.')` if already set up (`setup.js:42–63`).
4. `setupUser` (owner fixture, else `InternalServerError('Database missing fixture data...')`) → `doFixtures` (try/catch) → `doProductAndNewsletter` (try/catch) → `installTheme` → `doSettings` (title trimmed, description trimmed or `'Thoughts, stories and ideas.'`, accent_color if present) → `sendWelcomeEmail` (fire-and-forget).
5. Serializer `setup` → `{"users":[mappers.users(user,...)]}` (201).

**C++**

1. Public route (`routes.cppm:106-134`); `data.setup[0]` unwrap + name/email/password required (422) (`step:16-23`).
2. `assertSetupCompleted(false)` against the store (`'Setup has already been completed.'`, 403) (`step:25-26`).
3. Owner create/update with `status:'active'` and `roles:[Owner]` (`step:29-40`); fixture post `mobiledoc` edits and default tier/newsletter rename in the store (`:42-57`); theme install/activate and welcome mail recorded as outbox events (`:59-66`); title/description/accent_color written to the context store settings and config namespaces (`:68-71`).
4. Cleaned user model (password removed) in the exact serializer envelope `{"users":[...]}`, status 201 (`step:73-77`); handler emits `X-Cache-Invalidate: /*` (`handler.cppm:26-27`).

**Diff**

- Residual gaps: `database missing fixture data` branch (the context store creates the owner when none exists), GitHub theme download/activation adapter, SMTP welcome mail, `User.setup` internal-context permissions. Post-setup side effects are recorded in the store/outbox and are observable.
- Divergences: none at the endpoint wire contract. The former arbitrary-key settings dump and plaintext-persisting `{users,settings}` body are removed.

### PUT `/authentication/setup` — MATCH

**Ghost**

1. `mw.authAdminApi` (`routes.js:455`) = authenticate + authorize + last-seen + cors + redirects + prettyUrls + `tokenPermissionCheck` (which does not block this path).
2. Async `permissions` fn: find Owner, `owner.id !== frame.options.context.user` → `NoPermissionError('You are not the site owner.')` (`authentication.js:92–97`); validator `updateSetup` also requires `frame.options.context.user` (`input/setup.ts:13–18`).
3. `assertSetupCompleted(true)` → `setupUser` → `doSettings` → serializer `{"users":[mappers.users(user,...)]}`, 200, `X-Cache-Invalidate: /*`.

**C++**

1. The router trie stores one node per path, so the private verb is guarded by `authentication_route_detail::with_auth` (which calls `blog::handlers::middleware::require_auth`) invoked inside the `PUT` handler (`routes.cppm:20-33,106-134`); the public POST/GET verbs are untouched and the manual token check was removed from the handler. `core::auth::ghost::GhostAuthorizer` enforces the admin permission, and the store's owner id is compared with the request user (`step:107-126`), producing the exact 403 `NoPermissionError('You are not the site owner.')`.
2. `assertSetupCompleted(true)` (`:128-129`); owner row + title/description settings mutations (`:131-149`).
3. Cleaned user model in the exact serializer envelope, 200, `X-Cache-Invalidate: /*` (`step:151-155`, `handler.cppm:26-27`).

**Diff**

- Residual gaps: full `authAdminApi` stack (last-seen/cors/redirect/pretty-url middleware), integration-token allowlist nuances, `User.setup` internal-context handling.
- Divergences: none at the endpoint wire contract. The former raw settings-list body and header-only `X-User-Id` authorization are resolved.

### GET `/authentication/setup` — MATCH

**Ghost**

1. Public route (`routes.js:456`), `permissions:false`, `cacheInvalidate:false`.
2. `checkIsSetup()` → `User.isSetup()` (`models/user.js:900-902`: an Owner with status other than `inactive`) → `{status:true}`, else `{status:false,title:config.title,name:config.user_name,email:config.user_email}` from shared config.
3. Serializer `isSetup` → `{"setup":[data]}` (the e2e suite asserts `body.setup[0].status`, `test/e2e-api/admin/sso.test.js:107`).

**C++**

1. Public route (`routes.cppm:106-134`); no input (`step:14`).
2. `User.isSetup()`-equivalent store check (skips `inactive` users) (`step:15-17`); false branch reads the context-local `config` resource (then settings, then defaults) for `title`/`user_name`/`user_email` (`:18-26`).
3. Exact serializer envelope `{"setup":[...]}`, 200, no cache header (`step:28-29`, `handler.cppm:26-28`).

**Diff**

- Residual gaps: values originate from the context-local config/settings store rather than the process config module.
- Divergences: none at the endpoint wire contract.

### POST `/authentication/reset` — MATCH

**Ghost**

1. `mw.authAdminApi` (`routes.js:457`) with `tokenPermissionCheck`'s staff-token block: `NoPermissionError('Staff tokens are not allowed to access this endpoint')` (`middleware.js:9,34–43`); integration tokens blocked by allowlist (`:54–100`).
2. `permissions:true`; no validation block; `frame.options.context.user` is the actor.
3. `resetAuthentication` (`reset-authentication.ts:32–77`): `internalKeys.get('ghost-scheduler')`; transaction: `ApiKey.refreshAllSecrets`, `userService.lockAll`, `Action.add({event:'edited',resource_type:'security_action',context:{action_name:'reset_authentication',api_keys_rotated,users_locked}})`; then `internalKeys.clear()`, `deleteAllSessions()`, `schedulerAdapter.rescheduleAll({previousKey})`; returns `{apiKeysRotated,usersLocked}`. `destroyRequestSession` expires the client cookie (`:authentication.js:254`).
4. Serializer `reset` → `{"security_action":[{"action":"reset_authentication","api_keys_rotated":N,"users_locked":M}]}`, 200, `cacheInvalidate:false`.

**C++**

1. Per-verb `with_auth` guard (`routes.cppm:20-33,90-104`); staff tokens → 403 `NoPermissionError` with the exact message, integration tokens → 403 `'API tokens do not have permission to access this endpoint'`; `core::auth::ghost` enforces `permissions:true` (`step:16-29`).
2. Real context-store rotation of `api_keys` secrets and user `status:'locked'` with counts; audit row in `actions` with `action_name:'reset_authentication'` and both counts; `config` internal-key clear and session-row deletion (`step:31-58`).
3. `internal_keys.cleared`, `scheduler.reschedule_all`, and `session.destroyed` outbox events (`:60-70`).
4. Exact serializer envelope with live counts, 200, no cache header (`step:72-78`, `handler.cppm:26-28`).

**Diff**

- Residual gaps: `schedulerAdapter.rescheduleAll` and Express `req.session.destroy`/`Set-Cookie` expiry have no C++ primitive and are recorded on the outbox; process-wide API-key/internal-key singletons are represented by the context store.
- Divergences: none at the endpoint wire contract. The former `{users,reset_requests}` fixture snapshot is removed.

## Module-level notes

- All eight C++ steps keep `Input=interfaces::io::IRequest&`, `Output=std::map<std::string,std::string>` (the string-map form required by the workflow driver), a static `s_ctx` context pointer, and `[[nodiscard]]` results consumed or explicitly discarded. No new partition was added; shared helpers are private statics inside each step, so no central BUILD / all.cppm / base.cppm / central-route edit was needed.
- Inputs use `core::query::parse` for the allowlisted Ghost query options and a small local JSON-envelope unwrap for `data.<resource>[0]`; permission checks use `core::auth::ghost::GhostAuthorizer` and `blog::handlers::middleware::principal_from_request`; state lives in the context-local `ApiDataStore`; side effects go to the shared `blog::handlers::outbox`. `blog_handlers:mapping` is not wired into the module (module action-cycle blocker owned by the main agent), so no new cross-partition service module is imported.
- Cache handling follows the authoritative `@tryghost/api-framework/src/headers.ts`: `cacheInvalidate:false` emits **no** `X-Cache-Invalidate` header (six routes); `cacheInvalidate:true` emits `X-Cache-Invalidate: /*` (setup POST/PUT). The earlier report's literal `false`/`true` header values were wrong and are corrected here.
- The `authentication` output serializer (`output/authentication.js`) defines per-op envelopes that override the controller return values; all eight C++ steps emit those exact envelopes (including `setup:[...]`, `invitation:[{valid}]`, and `security_action:[...]`). The earlier report's prose ("mail send result", "{}", "void", "invite model") was imprecise and is corrected here.
- Auth middleware now mirrors `admin/routes.js`: no auth on the six public operations; `PUT /setup` and `POST /reset` invoke `require_auth` through a per-verb in-handler guard (one trie node per path), with staff/integration-token blocks and owner checks performed in-step via `core::auth::ghost`.
- Cross-route state: `password_reset_post` issues an expiring token in `password_reset_tokens` that `password_reset_put` consumes; setup POST writes settings/config that setup GET reads. Ghost's signed-token model is approximated by the stored issuing token (no `@tryghost/security` primitive).
- Residual side effects with no C++ primitive in this codebase: mail transport (SMTP/EmailError), HMAC/bcrypt cryptography, Knex transactions and `forUpdate` row locks, express-session rotation/cookie expiry, rate limiting (429), and GitHub theme install. These are cross-layer residuals, not route-level divergences — the same standard used to mark `behavior/previews.md` as MATCH.
