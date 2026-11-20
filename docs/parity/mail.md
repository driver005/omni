# Parity: mail

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/mail.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: none found
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

The controller exports a single operation, `send`, under `docName: 'mail'` (`mail.js:43-55`). It is registered in the API endpoint index as the getter `mail` (`api/endpoints/index.js:95-97`), and its output serializer is registered as `mail` → `./mail` (`api/endpoints/utils/serializers/output/index.js:83-85`). That serializer clones `frame.data`, deletes `mail[0].options` and `mail[0].message.transport`, sets `mail[0].status = {message: response.message}`, and assigns `frame.response` (`api/endpoints/utils/serializers/output/mail.js:4-18`).

No route in the admin URL table (or any other web `routes.js`) references either exported key (`mail` or `mail.send`). Grep for `api.mail`, `mail.send`, and `/mail` across `docs/Ghost/ghost/core/server/web/api/endpoints` and `docs/Ghost/ghost/core/server/web` returned 0 route bindings; the only `api.mail` hits across `docs/Ghost` are internal callers (`services/invites/invites.ts:201`, `api/endpoints/authentication.js:80,155`) plus the controller and its index registration. The controller is therefore internal-only in this Ghost snapshot: it defines `send` for programmatic use but no URL is wired to it. With zero Ghost routes, there is nothing to map to C++, and no C++ route to flag as extra.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| (none — no route binds `api.mail.send`) | `send` (`mail.js:46-54`) | none | none | Ghost `query(frame)` reads `frame.data`; `_private.sendMail` expects `object.mail[0].message` (`mail.js:18,52`); no `options[]` / validation object declared; no C++ side | Ghost output serializer `mail.all` clones `frame.data`, strips `mail[0].options` + `mail[0].message.transport`, adds `mail[0].status = {message}` (`output/mail.js:4-18`); no C++ side | Ghost `mailer.send(...)` via `mailService.GhostMailer` (no model/store call) and, on direct-send failure, `api.notifications.add` with internal context (`mail.js:13-40`); no C++ side | Ghost `headers.cacheInvalidate:false` → no invalidation (`mail.js:47-49`); no C++ side | Ghost `permissions:true` / `docName:'mail'` (`mail.js:44,50`); no HTTP auth middleware because no route; no C++ side | — | Ghost: `mail.js:13-55`; `index.js:95-97`; `output/index.js:83-85`; `output/mail.js:4-18`. Absence: grep `api.mail\|mail.send\|/mail` in `admin/routes.js` and all web `routes.js` → 0 route bindings. |

## Route details

No route details: the controller has no route in `admin/routes.js`, so there is no verb+path pair to audit and no both-sides file:line comparison to make. The per-check expectations above are recorded from the controller and its serializer only, for completeness.

## Missing in C++

None. `MISSING` requires a Ghost URL-table route with no C++ counterpart; this controller contributes no routes to the URL table. Confirmed by grep `api.mail`, `mail.send`, and `/mail` across `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` and `docs/Ghost/ghost/core/server/web` → 0 route bindings. The `api.mail` references that do exist (`services/invites/invites.ts:201`, `api/endpoints/authentication.js:80,155`) are internal programmatic calls, not HTTP routes.

## Extra in C++

None. No C++ folder, route, or handler exists for this domain. Confirmed by: no `mail*` directory under `include/blog/handlers/**/routes.cppm` (glob returned no files); grep `mail` across `include/blog/handlers` yields only `email`/`emails`/`automated_emails`/`email_previews` matches that belong to different Ghost controllers; no `MailRoutes` import or `install` call in `include/blog/handlers/routes.cppm` (`routes.cppm:3-56,70-130`); no mail route entry in `include/blog/testing/route_catalog.cppm` (only `/users/email/:email`, `/automations/*/email_*`, `/automated_emails*`, `/email_previews/*`, `/emails*` — `route_catalog.cppm:105,181-195,250-260`); and no mail handler import/accessor in `include/blog/handlers/all.cppm` (only `users_read_by_email`, `emails_*`, `email_previews_*`, `automated_emails_*`, and `automations_*_email_*` handlers, all belonging to other Ghost controllers). With no matched folder, there is no C++ route to classify as `EXTRA`.

## Fix

No change possible or needed. This controller contributes zero HTTP routes on either side: its only operation, `send`, is internal-only — there is no `router.<verb>('/mail...', ..., http(api.mail.send))` binding in the admin URL table or any web `routes.js` (grep `api.mail`, `mail.send`, `/mail` → 0 route bindings). The existing `api.mail` references are programmatic callers (`services/invites/invites.ts:201`, `api/endpoints/authentication.js:80,155`), not HTTP endpoints, and C++ has no mail route or handler to add, edit, or remove.

Because there is no URL-bound route or handler on either side, none of the permitted fix ingredients apply — no `blog_context`/`BlogContext` or `core_workflow` step to adjust, no `core_serde` DTO or `core_cache` entry to reconcile, no `core_router`/`utils/openapi` route registration, no `interfaces::io` request/response handling, and no `blog_handlers_middleware` chain. There is no missing C++ counterpart to build and no extra C++ route to gate. Fix count: 0. Do not invent work.
