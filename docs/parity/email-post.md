# Parity: email-post

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/email-post.js`
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

The controller exports a single operation, `read`, under `docName: 'email_post'` (`email-post.js:13,15`). It is registered in the API endpoint index as the getter `emailPost` (`api/endpoints/index.js:191-193`), and its output serializer is registered as `email_post` → `email-posts.js` (`utils/serializers/output/index.js:131-133`). That serializer wraps the returned model as `{email_posts:[...]}` (`utils/serializers/output/email-posts.js:30-34`).

No route in the admin URL table references either exported key (`emailPost` or `email_post`). Grep for `emailPost`, `email_post`, `uuid`, and `sent` across `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` returned 0 matches; a wider grep for `emailPost`. across `docs/Ghost/ghost/core/server` and for `emailPost`/`email_post` across all of `docs/Ghost` returned only the controller, its index registration, the output serializer, and unrelated frontend `email_posts` routing (`core/frontend/services/routing/config.ts:41`) — no `http(api.emailPost.read)` (or equivalent) binding. The controller is therefore orphaned in this Ghost snapshot: it defines `read` but no URL is wired to it. With zero Ghost routes, there is nothing to map to C++, and no C++ route to flag as extra.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| (none — no route binds `api.emailPost.read`) | `read` (`email-post.js:15`) | none | none | Ghost would accept `options[include∈{authors,tags,tiers}]`, `data[uuid required]`, `headers.cacheInvalidate:false`, `permissions:true` (`email-post.js:16-33`); no C++ side | Ghost `forceUrlRelations(frame,'posts')`, `Post.findOne(status:'sent')`, NotFoundError on miss, serializer `email_posts` (`email-post.js:34-49`; `email-posts.js:30-34`); no C++ side | Ghost `models.Post.findOne` with `status:'sent'` (`email-post.js:37-40`); no C++ side | Ghost `cacheInvalidate:false` → none (`email-post.js:16-18`); no C++ side | Ghost `permissions:true`/`docName:'email_post'` (`email-post.js:13,19`); no C++ side | — | Ghost: `email-post.js:13-51`; `index.js:191-193`; `output/index.js:131-133`. Absence: grep `emailPost\|email_post\|uuid\|sent` in `admin/routes.js` → 0 matches. |

## Route details

No route details: the controller has no route in `admin/routes.js`, so there is no verb+path pair to audit and no both-sides file:line comparison to make. The per-check expectations above are recorded from the controller only, for completeness.

## Missing in C++

None. `MISSING` requires a Ghost URL-table route with no C++ counterpart; this controller contributes no routes to the URL table. Confirmed by grep `emailPost`, `email_post`, `uuid`, `sent` in `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` → 0 matches, plus `emailPost` across `docs/Ghost/ghost/core/server/web` → 0 matches.

## Extra in C++

None. No C++ folder, route, or handler exists for this domain. Confirmed by: no `email_post`/`email-post` directory under `include/blog/handlers/**/routes.cppm`; grep `uuid|email_post|email-post` across `include/blog/handlers` → 0 matches; grep `email_post|email-post|emailPost` in `include/blog/handlers/routes.cppm` and `include/blog/testing/route_catalog.cppm` → 0 matches; and no `email_post` handler import/accessor in `include/blog/handlers/all.cppm` (only `emails_*`, `email_previews_*`, `automated_emails_*`, and `automations_*_email_*` handlers, which belong to different Ghost controllers). With no matched folder, there is no C++ route to classify as `EXTRA`.
