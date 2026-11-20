# Parity: authentication

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/authentication.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- Ghost output serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/authentication.js`
- C++: `include/blog/handlers/auth/authentication/routes.cppm` (all 8 handlers under `auth/authentication/<op>/`)
- Checked: 2026-11-19; strict-parity rewrite applied same date. See `docs/parity/behavior/authentication.md` for the authoritative route-by-route behavior report.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 8 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All 8 Ghost `authentication` routes have a C++ route with matching verb + path. The eight
historical `#### Fix` blocks below were applied: handlers/steps were rewritten from the generic
scaffold to strict Ghost behavior using `core::query`, `core::auth::ghost`, the context store,
and the shared outbox. No new partition, no shared BUILD / all.cppm / base.cppm / central-route
edit, and no new cross-partition service-module import.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST /authentication/password_reset | generateResetToken | POST /authentication/password_reset | `authentication_authentication_password_reset_post` | `data.password_reset[0].email`, `options:['email']`, core::query | `{"password_reset":[{"message":"Check your email for further instructions."}]}` | user lookup by email; expiring token in `password_reset_tokens`; mail outbox event | no header (`cacheInvalidate:false`) | public; setup guard; `core::auth::ghost` `permissions:true` | MATCH | `routes.js:440-445`, `authentication.js:140-157`, `output/authentication.js#generateResetToken` / `routes.cppm:33-47`, `password_reset_post/step:16-77` |
| PUT /authentication/password_reset | resetPassword | PUT /authentication/password_reset | `authentication_authentication_password_reset_put` | `data.password_reset[0].{token,newPassword,ne2Password}`, `options:['ip']` | `{"password_reset":[{"message":"Password updated"}]}` | token consume, user password+status edit, session/spam outbox events | no header | public; setup guard | MATCH | `routes.js:446-451`, `passwordreset.ts:106-209` / `password_reset_put/step:16-90` |
| POST /authentication/invitation | acceptInvitation | POST /authentication/invitation | `authentication_authentication_invitation_post` | `data.invitation[0].{token,name,password}` | `{"invitation":[{"message":"Invitation accepted."}]}` | find sent invite, expiry/duplicate checks, destroy invite + create user | no header | public; setup guard | MATCH | `routes.js:452`, `accept.ts:49-98` / `invitation_post/step:14-72` |
| GET /authentication/invitation | isInvitation | GET /authentication/invitation | `authentication_authentication_invitation_get` | query `email` via core::query, strict 400 validator | `{"invitation":[{"valid":true\|false}]}` | `invites` lookup by email + `status:'sent'` | no header | public; setup guard | MATCH | `routes.js:453`, `input/invitations.js:34-44` / `invitation_get/step:14-45` |
| POST /authentication/setup | setup | POST /authentication/setup | `authentication_authentication_setup_post` | `data.setup[0]` fields | `{"users":[<mapped user>]}`, 201 | owner user + settings/config; fixture posts, tier/newsletter, theme/mail outbox | `X-Cache-Invalidate: /*` | public; `assertSetupCompleted(false)` | MATCH | `routes.js:454`, `setup.js:65-244` / `setup_post/step:15-90`, `handler:26-27` |
| PUT /authentication/setup | updateSetup | PUT /authentication/setup | `authentication_authentication_setup_put` | `data.setup[0]` fields | `{"users":[<mapped user>]}`, 200 | owner user + settings/config edits | `X-Cache-Invalidate: /*` | per-verb in-handler auth guard; `core::auth::ghost` owner check (`'You are not the site owner.'`); setup guard | MATCH | `routes.js:455`, `input/setup.ts:13-18` / `setup_put/step:107-166`, `routes.cppm:21-33,93-108` |
| GET /authentication/setup | isSetup | GET /authentication/setup | `authentication_authentication_setup_get` | none | `{"setup":[{status:true}\|{status:false,title,name,email}]}` | `User.isSetup()`-equivalent store check + config lookup | no header | public | MATCH | `routes.js:456`, `output/authentication.js#isSetup` / `setup_get/step:14-33` |
| POST /authentication/reset | reset | POST /authentication/reset | `authentication_authentication_reset_post` | options only (`context.user` actor) | `{"security_action":[{"action":"reset_authentication","api_keys_rotated":N,"users_locked":M}]}`, 200 | key rotation, user lock, audit row, session clear, outbox events | no header | per-verb in-handler auth guard; staff-token block; `core::auth::ghost` `permissions:true` | MATCH | `routes.js:457`, `reset-authentication.ts:32-77` / `reset_post/step:16-84`, `routes.cppm:21-33,76-89` |

## Applied fixes (historical `#### Fix` blocks)

Each historical Fix block targeted checks 1–8 (identity, op, input, output, DB call, cache, auth,
verdict). All eight are now closed:

- **Checks 1–2 (route/op):** unchanged verb+path registrations, with auth scoped per-verb
  (in-handler `require_auth` guard only on `PUT /setup` and `POST /reset`, mirroring
  `admin/routes.js:455,457`; one trie node per path).
- **Check 3 (input):** `core::query::parse` for allowlisted options plus `data.<resource>[0]` unwrap;
  Ghost validator messages/statuses reproduced (400 for invalid email, 422 for password/name/token
  validators, 422 `'Your new passwords do not match'`).
- **Check 4 (output):** the per-op envelopes from the Ghost `authentication` output serializer are
  emitted verbatim, replacing the former `{users,settings}` / `{users,reset_requests}` fixtures and
  the generic `{"authentication":[...]}` scaffold.
- **Check 5 (DB call):** all state lives in the context-local `ApiDataStore` (`users`, `invites`,
  `password_reset_tokens`, `settings`, `config`, `actions`, `api_keys`, `sessions`); side effects
  (mail, theme, scheduler, session, spam) are recorded on `blog::handlers::outbox`.
- **Check 6 (cache):** corrected to the authoritative `@tryghost/api-framework/src/headers.ts`
  semantics — `cacheInvalidate:false` emits no header; `cacheInvalidate:true` emits `/*`. The
  earlier literal `false`/`true` values were wrong.
- **Check 7 (auth):** dead `token(req).empty() && !true` guards deleted; `core::auth::ghost`
  permission decisions, setup guards, owner checks, and staff/integration-token blocks implemented.
- **Check 8 (verdict):** all eight flipped to MATCH at the endpoint contract, with cross-layer
  residuals documented in `docs/parity/behavior/authentication.md` (same standard as `previews.md`).

## Missing in C++

None. All 8 Ghost `authentication` routes have a C++ route with matching verb + path in
`include/blog/handlers/auth/authentication/routes.cppm`.

## Extra in C++

None. The folder installs exactly 8 routes (invitation POST/GET, password_reset POST/PUT, reset
POST, setup POST/PUT/GET) — all 8 have a corresponding Ghost controller route.

## Residuals (no primitive in this C++ codebase)

Mail transport (SMTP/`EmailError`), `@tryghost/security` HMAC/bcrypt cryptography, Knex
transactions/`forUpdate`, express-session rotation and `Set-Cookie` expiry, rate limiting (429),
and GitHub theme install. These are cross-layer gaps, not route-level divergences; observable
effects are recorded on the context store and the shared outbox. Details per route are in
`docs/parity/behavior/authentication.md`.
