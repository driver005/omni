# Behavior report: mail

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/mail.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (no binding — see Module-level notes)
- C++ folders: none (no `mail*` route/handler exists)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Module-level finding only. The controller exports one operation (`send`) under `docName: 'mail'` but **no HTTP route binds it** in this Ghost snapshot. With zero Ghost routes there is nothing to map to C++, and with no C++ mail route there is nothing to flag as `EXTRA`. `MISSING` requires a Ghost URL-table route with no C++ counterpart; this controller contributes none.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| (none — no route binds `api.mail.send`) | `send` (`mail.js:46-54`, docName `mail` at `:44`) | `query(frame)` reads `frame.data`; `_private.sendMail` expects `object.mail[0].message` (`mail.js:18,52`); no `options[]`, no `validation{}`, no path params, no multipart | `permissions: true` (`mail.js:50`); no HTTP middleware because no route; internal callers pass `{context:{internal:true}}` (`invites.ts:201`) | `headers.cacheInvalidate:false` (`mail.js:47-49`); no `statusCode`, no custom headers (`Content-Disposition`/`X-Cache-Invalidate`/`Location` absent) | `send.query` → `_private.sendMail(frame.data)` (`mail.js:52`) → lazy `GhostMailer` singleton (`:14-16`) → `mailer.send(object.mail[0].message)` (`:18`); on failure when `mailer.state.usingDirect`, `api.notifications.add` warn with internal context (`:19-36`), then rethrow (`:38`) | no invalidation (`cacheInvalidate:false`); no cache service/key/TTL | serializer `mail.all` (`output/mail.js:4-18`): `_.cloneDeep(frame.data)`, delete `mail[0].options` + `mail[0].message.transport`, set `mail[0].status={message: response.message}`, assign `frame.response` | none | none | — | Ghost: `mail.js:13-55`; `api/endpoints/index.js:95-97`; `utils/serializers/output/index.js:83-85`; `output/mail.js:4-18`; `lib/mail/ghost-mailer.js:117-141,161-202`. Absence: grep `api.mail`, `mail.send`, `/mail` across `web/api/endpoints/admin/routes.js` and all web `routes.js` → 0 route bindings. C++: no `mail*` under `include/blog/handlers`; `handlers/routes.cppm:84-91` installs only `AutomatedEmailsRoutes`/`EmailPreviewsRoutes`/`EmailsRoutes`. |

## Route details

No route details: the controller has no verb+path pair in `admin/routes.js`, so there is no both-sides comparison to make. The eight checklist expectations are recorded from the controller and its serializer in the table above for completeness.

### Module-level finding — `send` with no HTTP binding

**Ghost** (file:line):

1. Identity: no verb + path. `admin/routes.js` contains no `http(api.mail...)` entry; grep for `api.mail`, `mail.send`, and `/mail` across `web/api/endpoints/admin/routes.js` and all web `routes.js` returned 0 route bindings. The `mail` string hits in `admin/routes.js` are unrelated email endpoints (`/users/email/:email` at `:129`, `/automations/:id/email_preview` at `:317-319`, `/email_previews/posts/:id` at `:547-553`, `/emails*` at `:557-565`).
2. Operation: sole key `send`, `docName: 'mail'` (`mail.js:43-55`); registered as API getter `mail` → `apiFramework.pipeline(require('./mail'), localUtils)` (`api/endpoints/index.js:95-97`).
3. Input: `query(frame)` passes `frame.data` to `_private.sendMail` (`mail.js:51-53`); the mailer is invoked with `object.mail[0].message` (`:18`). No `options[]` list, no `validation{}` object, no body shape declaration, no query aliases, no headers read from the request.
4. Auth/permissions: `permissions: true` (`mail.js:50`) — a framework-level declaration, but with no route there is no `mw.authAdminApi`/member middleware in a chain. Internal callers pass `{context: {internal: true}}` (`services/invites/invites.ts:201`).
5. Status/headers: `headers: { cacheInvalidate: false }` (`mail.js:47-49`). No `statusCode`, no custom response headers, no `format`/`stream` override.
6. Full call chain: `query(frame)` → `_private.sendMail(frame.data)` (`:52`). Lazy singleton `mailer` created only if not `instanceof mailService.GhostMailer` (`:14-16`). `mailer.send(object.mail[0].message)` (`:18`) → `GhostMailer.send` (`lib/mail/ghost-mailer.js:161-202`): validates `subject`/`html`/`to` (`:162-167`), `createMessage` resolves from/reply-to (`:169`, `:41-60`), transport `sendMail` (`:195`, `:204-242`), direct-transport response handling (`:197-199`, `:244-262`). On rejection: if `mailer.state.usingDirect` (`:19`), `api.notifications.add` with type `warn` and a two-part message (unable-to-send + docs link, `messages` at `:7-11`, `:20-35`) under `{context:{internal:true}}` (`:34`); error always rethrown (`:38`). `GhostMailer.state` set from config transport `direct`/`mailgun`/`smtp` (`ghost-mailer.js:121-140`). Side effect: real SMTP/Mailgun/direct send; no model/store call, no webhook, no job queue in the controller.
7. Cache: none. `cacheInvalidate:false` → no invalidation; no cache service, key, namespace, tag, or TTL.
8. Output: serializer `mail.all` (`utils/serializers/output/mail.js:4-18`, registered via `output/index.js:83-85`). Deep-clones `frame.data`, strips `mail[0].options` and `mail[0].message.transport`, sets `mail[0].status = {message: response.message}`, assigns `frame.response`. Empty/missing `mail[0]` would throw in the serializer; no explicit empty-result branch.

**Internal (non-route) callers of `api.mail`** — evidence the controller is programmatic-only:

- `api/endpoints/authentication.js:80` — `auth.setup.sendWelcomeEmail(user.get('email'), api.mail)`.
- `api/endpoints/authentication.js:155` — `auth.passwordreset.sendResetNotification(token, api.mail)`.
- `services/invites/invites.ts:201` — `api.mail.send(payload, {context: {internal: true}})`.

These are service-to-controller invocations, not `router.<verb>(...)` URL bindings, so they do not create a Ghost route to compare.

**C++** (file:line): no counterpart exists.

- No `mail*` directory under `include/blog/handlers/**` (glob returned no files).
- `include/blog/handlers/BUILD.bazel` lists only `email`, `emails`, `email_previews`, `automated_emails`, and `automations/*email_*` files — all for other Ghost controllers; no `mail` handler.
- `include/blog/handlers/routes.cppm:12,18-19` imports only `:automated_emails_routes`, `:email_previews_routes`, `:emails_routes`; `:84-91` installs `AutomatedEmailsRoutes`, `EmailPreviewsRoutes`, `EmailsRoutes`. No `MailRoutes`.
- Grep `mail` across `include/blog/handlers` yields only unrelated `email`/`EmailWire`/`mailgun_message_id` fixtures, not a `mail.js` controller mapping.

**Diff**: residual gaps (Ghost behaviors with no C++ primitive) are moot — with no Ghost route there is no surface to port. The `GhostMailer` transport send, direct-send warn-notification side effect, and `mail.all` serializer are not represented in C++, but there is no C++ endpoint that should host them. No divergences: no C++ handler fabricates a mail route or stand-in behavior.

## Module-level notes

- The `mail` controller is the internal mail-send primitive used by setup, password reset, and invites. It is deliberately not exposed over HTTP in this snapshot; its `permissions:true`/`docName:'mail'` are framework metadata that would apply only if a route were wired.
- `send` is lazy-singleton: one `GhostMailer` instance reused across calls (`mail.js:4,14-16`), constructed with transport from config (`ghost-mailer.js:121-140`).
- The only failure side effect is the direct-transport warn notification (`mail.js:19-36`); the error is always rethrown, so callers see the transport error.
- Nothing to fix: zero Ghost HTTP routes and zero C++ mail routes. Do not invent a route or handler.
