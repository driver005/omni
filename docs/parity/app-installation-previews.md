# Parity: app-installation-previews

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/app-installation-previews.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: none found (only unrelated `include/blog/handlers/external/apps/routes.cppm` GET `/apps`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 1 |
| EXTRA | 1 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| `POST /apps/installations/preview` | `add` | — (none) | — (none) | Ghost: body `app_installation_previews[0].manifest_url` (zod, `z.string()`). C++: none | Ghost: service result, no serializer/statusCode/headers override. C++: none | Ghost: `appInstallations.service.preview(manifest_url)` — no model/store call, nothing stored. C++: none | Ghost: `headers.cacheInvalidate: false`. C++: none | Ghost: `mw.authAdminApi` + labs `apps` + permissions `docName: app_installations`, `method: add`. C++: none | MISSING | Ghost: `.../admin/routes.js:232`, `.../app-installation-previews.ts:12-28`. C++ absence: grep `installations` over `include/blog/handlers/**` → 0 matches; grep `manifest_url\|installations/preview\|app_installation\|appInstallation` over `include` → only `include/blog/context/context.cppm:122-123,226-227` (field defaults, no route) |

## Route details

### `POST /apps/installations/preview` — MISSING

- Route identity: Ghost binds `appsRouter.post('/installations/preview', http(api.appInstallationPreviews.add))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:232`. Mounted under `/apps` via `router.use('/apps', appsRouter)` (`:229`). No C++ `Route{...}` or `Router{...}` binds `/apps/installations/preview`; C++ apps router binds only `.get(...)` on `/apps` at `include/blog/handlers/external/apps/routes.cppm:19-27`.
- Operation mapping: Ghost op `add` (`app-installation-previews.ts:18-28`), exported via camelCase getter `appInstallationPreviews` in `.../api/endpoints/index.js:355-356`. No C++ `handler.<method>()` for any app-installation preview op: `include/blog/handlers/all.cppm` has no such handler (its only preview handlers are `stripe_stripe_checkout_preview_post`, `automations_automations_id_email_preview_post`, `automated_emails_automated_emails_id_preview_post`, `email_previews_*` — `all.cppm:84,140,153,208-209`).
- Input parity: Ghost `PreviewFrame` requires `data.app_installation_previews` (non-empty array enforced by framework per `app-installation-previews.ts:6-8`) and each element validated with `PreviewRequest = z.object({ manifest_url: z.string() })` (`:10`, `:22`); failure throws `ValidationError` (`:23-25`). C++ declares no `using Input` for this operation — none exists.
- Output parity: Ghost `query()` returns `appInstallations.service.preview(...)` result directly (`:26`); no `statusCode`, no `response.format`/serializer, no `stream`. C++ has no output envelope for this operation.
- DB call parity: Ghost calls service `appInstallations.service.preview(parsed.data.manifest_url)` (`:26`) — fetches a remote manifest, stores nothing (`:15-17`). C++ has no `store().json_for(...)` / `find_by_resource` / mutation for this; nearest app-related store call is `ctx.store().json_for("apps", {})` in `include/blog/handlers/external/apps/all_collection/workflow/step/execute.cppm:15`, which is a different route and resource.
- Cache parity: Ghost sets `headers: { cacheInvalidate: false }` (`:19`) — no invalidation. C++: none for this operation.
- Auth / permissions: Ghost route guarded by `router.all(['/apps', '/apps/*'], mw.authAdminApi)` (`routes.js:228`) and `appsRouter.use(labs.enabledMiddleware('apps'))` (`:230`); controller declares `permissions: { docName: 'app_installations', method: 'add' }` (`:20`). C++: none for this operation.
- Verdict: MISSING — Ghost route exists; no C++ route/handler anywhere in `include/blog/handlers/**` matches path, verb, or operation.

## Missing in C++

- `POST /apps/installations/preview` → Ghost `add` (verdict MISSING). Evidence: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:232`; `docs/Ghost/ghost/core/server/api/endpoints/app-installation-previews.ts:12-28`. Confirming searches: `grep -r "installations" include/blog/handlers` → 0 hits; `grep -r "manifest_url\|installations/preview\|app_installation\|appInstallation" include` → only `include/blog/context/context.cppm:122-123,226-227` (schema field metadata, not a route); `include/blog/handlers/all.cppm` contains no app-installation-preview handler import/member/accessor; `include/blog/handlers/routes.cppm:77` installs only `AppsRoutes`, whose sole binding is GET `/apps`.

## Extra in C++

- `GET /apps` → `handler.apps_all_collection()` at `include/blog/handlers/external/apps/routes.cppm:15-29` (bound at `:19`, `:26`). No Ghost route references it: the corresponding Ghost controller `app-installation-previews.ts` defines only `add` for `POST /apps/installations/preview`, and `routes.js:227-238` defines no `GET /apps` (only `/apps/installations*`). Note: this appears to target a different/legacy Ghost domain (`apps` collection), but within the audited controller it has no counterpart. Its step (`include/blog/handlers/external/apps/all_collection/workflow/step/execute.cppm:15-16`) returns `{"apps":[...], "meta":{"pagination":{...}}}` from `ctx.store().json_for("apps", {})`.
